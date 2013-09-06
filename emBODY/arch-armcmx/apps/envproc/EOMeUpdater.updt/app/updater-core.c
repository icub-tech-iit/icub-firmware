/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Alessandro Scalzo
 * email:   alessandro.scalzo@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include "eupdater_parser.h"


#include "updater-core.h"
#include "eupdater_cangtw.h"

#include "stdlib.h"
#include "string.h"

#include "hal.h"
#include "osal.h"
#include "ipal.h"

#include "shalBASE.h"
#include "shalPART.h"
#include "shalINFO.h"
 

typedef enum {
    CMD_SCAN            = 0xFF,
    CMD_START           = 0x01,
    CMD_DATA            = 0x02,
    CMD_JUMP            = 0x03,
    CMD_END             = 0x04,
    CMD_BOOT            = 0x05,
    CMD_RESET           = 0x06,
    CMD_IPSET           = 0x07,
    CMD_MASKSET         = 0x08,
    CMD_PROCS           = 0x09,
    CMD_SHALS           = 0x0A,
    CMD_BLINK           = 0x0B,
    CMD_UPD_ONCE        = 0x0C,
    CMD_MACGET          = 0x10,
    CMD_MACSET          = 0x11,
    CMD_SYSEEPROMERASE  = 0x12,
    CMD_CANGTW_START    = 0x20,
    CMD_CANGTW_STOP     = 0x21,
} canloader_opcode_t;

enum {
    UPD_OK        = 0,
    UPD_ERR_PROT  = 1,
    UPD_ERR_FLASH = 2,
    UPD_ERR_LOST  = 3,
    UPD_ERR_UNK   = 4,
    UPD_ERR_OOM   = 5
};

#define BOARD_TYPE_EMS 0x0A


// static functions
static eEresult_t s_sys_eeprom_erase(void);

void upd_core_init(void)
{
    hal_gpio_init(hal_gpio_portE, hal_gpio_pin13, hal_gpio_dirOUT, hal_gpio_speed_low); 
}

#define PROGRAM_LOADER   0x55
#define PROGRAM_UPDATER  0xAA
#define PROGRAM_APP      0x5A

uint8_t upd_core_manage_cmd(uint8_t *pktin, eOipv4addr_t remaddr, uint8_t *pktout, uint16_t *sizeout)
{
    static uint32_t s_prog_mem_start = 0xFFFFFFFF;
    static uint32_t s_prog_mem_size  = 0;
    static uint16_t s_download_packets = 0;
    static uint8_t  s_download_state = 0;

    *sizeout=0;


        
#if defined(_DEBUG_MODE_)
    #warning --> we are in _DEBUG_MODE_ and use: POS_OPC = 1
    #define POS_OPC 1
#else
    #define POS_OPC 0
#endif
    
    switch(pktin[POS_OPC]) // opcode
    {
        case CMD_CANGTW_START:
        {
#if !defined(_MAINTAINER_APPL_ )        
            // if updater we also start the can gateway
            eupdater_cangtw_start(remaddr);
#endif               
            return 0;
        }// break;
        
        case CMD_SCAN:
        {
            //hal_trace_puts("CMD_SCAN");

            eEmoduleInfo_t* module=(eEmoduleInfo_t*)(EENV_MEMMAP_EUPDATER_ROMADDR+EENV_MODULEINFO_OFFSET);

            *sizeout = 14;

            pktout[ 0] = CMD_SCAN;
            pktout[ 1] = module->info.entity.version.major;
            pktout[ 2] = module->info.entity.version.minor;
            pktout[ 3] = BOARD_TYPE_EMS;
            
            const eEipnetwork_t *ipnetworkstrg;
            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);

            pktout[ 4] = (ipnetworkstrg->ipnetmask>>24) & 0xFF;
            pktout[ 5] = (ipnetworkstrg->ipnetmask>>16) & 0xFF;
            pktout[ 6] = (ipnetworkstrg->ipnetmask>>8)  & 0xFF;
            pktout[ 7] =  ipnetworkstrg->ipnetmask      & 0xFF;

            pktout[ 8] = (ipnetworkstrg->macaddress>>40) & 0xFF;
            pktout[ 9] = (ipnetworkstrg->macaddress>>32) & 0xFF;
            pktout[10] = (ipnetworkstrg->macaddress>>24) & 0xFF;
            pktout[11] = (ipnetworkstrg->macaddress>>16) & 0xFF;
            pktout[12] = (ipnetworkstrg->macaddress>>8)  & 0xFF;
            pktout[13] = (ipnetworkstrg->macaddress)     & 0xFF;

            return 1;
        }// break;

        case CMD_START:
        {
            //hal_trace_puts("CMD_START");

            s_download_packets = 1;
            s_download_state = pktin[1];
            
            *sizeout = 2;
            pktout[0] = CMD_START;

            switch (s_download_state)
            {
            case PROGRAM_LOADER:
                s_prog_mem_start = EENV_MEMMAP_ELOADER_ROMADDR;
                s_prog_mem_size  = EENV_MEMMAP_ELOADER_ROMSIZE;
                break;

            case PROGRAM_UPDATER:
                s_prog_mem_start = EENV_MEMMAP_EUPDATER_ROMADDR;
                s_prog_mem_size  = EENV_MEMMAP_EUPDATER_ROMSIZE;
                break;

            case PROGRAM_APP:
                s_prog_mem_start = EENV_MEMMAP_EAPPLICATION_ROMADDR;
                s_prog_mem_size  = EENV_MEMMAP_EAPPLICATION_ROMSIZE;
                break;

            default:
                s_download_state = 0;
                pktout[1] = UPD_ERR_UNK;
                return 1;    
            }
            
            osal_system_scheduling_suspend();
                
            if (hal_res_OK != hal_flash_erase(s_prog_mem_start, s_prog_mem_size))
            {
                s_download_state = 0;
                //hal_trace_puts("ERASE FAILED");
                pktout[1] = UPD_ERR_FLASH;    
            }
            else
            {
                //hal_trace_puts("ERASE DONE");
                pktout[1] = UPD_OK;
            }
            
            osal_system_scheduling_restart();
            
            if(0 != s_download_state)
            {
                // download is to begin, thus start blinking led in a different way
                eupdater_parser_download_blinkled_start();
            }
            
            return 1;
        }// break;
            
        case CMD_DATA:
        {
            if (!s_download_state)
            {
                return 0;
            }

            //hal_trace_puts("CMD_DATA");

            *sizeout = 2;
            pktout[0] = CMD_DATA;

            uint32_t address = pktin[4]<<24 |
                               pktin[3]<<16 |
                               pktin[2]<<8  |
                               pktin[1];

            uint16_t size = pktin[6]<<8 | pktin[5];

            void *data = pktin + 7;
                  
            if ((address >= s_prog_mem_start) && (address+size < s_prog_mem_start+s_prog_mem_size))
            {
                hal_sys_irq_disable();
                hal_flash_unlock();
                hal_flash_write(address, size, data);
                hal_sys_irq_enable();

                ++s_download_packets;

                pktout[1] = UPD_OK;
            }
            else
            {
                pktout[1] = UPD_ERR_PROT;
            }
            
            eupdater_parser_download_toggleled();   
               
            return 1;
        }// break;
            
        case CMD_END:
        {
            if (!s_download_state)
            {
                return 0;
            }

            //hal_trace_puts("CMD_END");

            *sizeout = 2;
            pktout[0] = CMD_END;

            if ((pktin[2]<<8)|pktin[1] == s_download_packets)
            {
                pktout[1] = UPD_OK;

                switch (s_download_state)
                {
                case PROGRAM_LOADER:
                    shalpart_proc_synchronise(ee_procLoader,(eEmoduleInfo_t*)(EENV_MEMMAP_ELOADER_ROMADDR+EENV_MODULEINFO_OFFSET));
                    break;    
                case PROGRAM_UPDATER:
                    shalpart_proc_synchronise(ee_procUpdater,(eEmoduleInfo_t*)(EENV_MEMMAP_EUPDATER_ROMADDR+EENV_MODULEINFO_OFFSET));
                    shalpart_proc_def2run_set(ee_procApplication);
                    break;
                case PROGRAM_APP:
                    shalpart_proc_synchronise(ee_procApplication,(eEmoduleInfo_t*)(EENV_MEMMAP_EAPPLICATION_ROMADDR+EENV_MODULEINFO_OFFSET));
                    shalpart_proc_def2run_set(ee_procUpdater);
                    break;
                }
            }
            else
            {
                pktout[1] = UPD_ERR_LOST;

                if (s_download_state != PROGRAM_LOADER)
                {
                    osal_system_scheduling_suspend();
                    hal_flash_erase(EENV_MEMMAP_EAPPLICATION_ROMADDR, EENV_MEMMAP_EAPPLICATION_ROMSIZE);
                    osal_system_scheduling_restart();
                }
            }
   
            s_download_state = 0;

            // download is over, thus resume the normal blinking of led
            eupdater_parser_download_blinkled_stop();

            return 1;
        }// break;

        case CMD_BOOT:
        {
            //hal_trace_puts("CMD_BOOT");

            eEprocess_t active_part = (eEprocess_t)pktin[1];

            if ((active_part >= ee_procUpdater) && (active_part <= ee_procApplUser04))
            {
                shalpart_proc_def2run_set(active_part);
            }

            return 0;
        }// break;

        case CMD_RESET:
        {
            //hal_trace_puts("CMD_RESET");
            shalbase_system_restart();
            
            return 0;
        }// break;

        case CMD_IPSET:
        {
            //hal_trace_puts("CMD_IPSET");

            uint8_t *ip = pktin;
            
            eEipnetwork_t ipnetwork;
            const eEipnetwork_t *ipnetworkstrg;

            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);
            memcpy(&ipnetwork, ipnetworkstrg, sizeof(eEipnetwork_t));
            ipnetwork.ipaddress = EECOMMON_ipaddr_from(ip[1], ip[2], ip[3], ip[4]);
            
            shalinfo_deviceinfo_part_set(shalinfo_ipnet, (const void*)&ipnetwork);

            return 0;
        }// break;

        case CMD_MASKSET:
        {
            //hal_trace_puts("CMD_MASKSET");
            
            uint8_t *ip = pktin;
            
            eEipnetwork_t ipnetwork;
            const eEipnetwork_t *ipnetworkstrg;

            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);
            memcpy(&ipnetwork, ipnetworkstrg, sizeof(eEipnetwork_t));
            ipnetwork.ipnetmask = EECOMMON_ipaddr_from(ip[1], ip[2], ip[3], ip[4]);
            shalinfo_deviceinfo_part_set(shalinfo_ipnet, (const void*)&ipnetwork);

            return 0;
        }// break;
        
        case CMD_MACGET:
        {                     
            const eEipnetwork_t *ipnetworkstrg;
            
            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);
            pktout[0] = CMD_MACGET;
            memcpy(&pktout[1], &ipnetworkstrg->macaddress, sizeof(ipnetworkstrg->macaddress));      
            
            return 1;
        }// break;        
        
        case CMD_MACSET:
        {
                     
            eEipnetwork_t ipnetwork;
            const eEipnetwork_t *ipnetworkstrg;
            uint64_t mac = 0;
            memcpy(&mac, &pktin[1], 6);

            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);
            memcpy(&ipnetwork, ipnetworkstrg, sizeof(eEipnetwork_t));
            ipnetwork.macaddress = mac;
            shalinfo_deviceinfo_part_set(shalinfo_ipnet, (const void*)&ipnetwork);
            
            char str[128];
            snprintf(str, sizeof(str), "mac is h = %llx, l = %llx", ipnetwork.macaddress >> 32, ipnetwork.macaddress & 0xffffffff);
            hal_trace_puts(str);    
            
            shalinfo_deviceinfo_part_get(shalinfo_ipnet, (const void**)&ipnetworkstrg);
            pktout[0] = CMD_MACSET;
            memcpy(&pktout[1], &ipnetworkstrg->macaddress, sizeof(ipnetworkstrg->macaddress));      
            
            return 1;
        }// break;
                
        case CMD_SYSEEPROMERASE:
        {   
            eEresult_t ret = s_sys_eeprom_erase();

            pktout[0] = EENV_MEMMAP_SHALSYSTEM_STGADDR;
            pktout[1] = (ee_res_OK == ret) ? 1 : 0;
            
            return 1;
        }// break;   
        
        case CMD_PROCS:
        {
            uint8_t num_procs = 0;
            const eEprocess_t *s_proctable = NULL;
            const eEmoduleInfo_t *s_modinfo = NULL;
            eEprocess_t defproc;

            shalpart_proc_def2run_get(&defproc);

            if (ee_res_OK == shalpart_proc_allavailable_get(&s_proctable, &num_procs))
            {
                pktout[0] = CMD_PROCS;
                pktout[1] = num_procs; 

                char *data = (char*)pktout;
                uint16_t size = 2;

                for (uint8_t i=0; i<num_procs; ++i)
                {
                    shalpart_proc_get(s_proctable[i], &s_modinfo);

                    size+=sprintf(data+size,"*** e-proc #%d %s ***\r\n",i,(defproc==i?"(default)":""));

                    size+=sprintf(data+size, "name\t%s\r\n", s_modinfo->info.name);
                    size+=sprintf(data+size, "version\t%d.%d %d/%d/%d %d:%.2d\r\n", 
                        s_modinfo->info.entity.version.major, 
                        s_modinfo->info.entity.version.minor,
                        s_modinfo->info.entity.builddate.day,
                        s_modinfo->info.entity.builddate.month,
                        s_modinfo->info.entity.builddate.year,
                        s_modinfo->info.entity.builddate.hour,
                        s_modinfo->info.entity.builddate.min
                    );
                    size+=sprintf(data+size, "rom.addr\t0x%0.8X\r\n", s_modinfo->info.rom.addr);
                    size+=sprintf(data+size, "rom.size\t0x%0.8X\r\n", s_modinfo->info.rom.size);
                    size+=sprintf(data+size, "ram.addr\t0x%0.8X\r\n", s_modinfo->info.ram.addr);
                    size+=sprintf(data+size, "ram.size\t0x%0.8X\r\n", s_modinfo->info.ram.size);

                    size+=sprintf(data+size, "stg.type\t%s\r\n", (ee_strg_none == s_modinfo->info.storage.type) ? ("none") 
                                                                : ((ee_strg_eflash==s_modinfo->info.storage.type) ? ("flash") : ("eeprom")));
                    size+=sprintf(data+size, "stg.addr\t0x%0.8X\r\n", s_modinfo->info.storage.addr);
                    size+=sprintf(data+size, "stg.size\t0x%0.8X\r\n", s_modinfo->info.storage.size);
                    size+=sprintf(data+size, "com.msk\t0x%0.8X\r\n\r\n", s_modinfo->info.communication);
                }

                *sizeout = size + 1;
            }

            return 1;
        }// break
        /*
        case CMD_SHALS:
        {
            eEresult_t shalres;
            uint8_t num_shals = 3;
            const eEmoduleInfo_t *s_modinfo = NULL;
            const eEsharlib_t s_shartable[] = {ee_shalBASE, ee_shalPART, ee_shalINFO};  
            
            pktout[0] = CMD_SHALS;
            pktout[1] = num_shals; 

            char *data = (char*)pktout;
            uint16_t size = 2;

            for (uint8_t i=0; i<num_shals; ++i)
            {
                shalres = shalpart_shal_get(s_shartable[i], &s_modinfo);

                size+=sprintf(data+size,"*** shal #%d ***\r\n",i);

                if (shalres != ee_res_OK)
                {
                    size+=sprintf(data+size,"NOT PRESENT\r\n\r\n");
                    continue;
                }

                size+=sprintf(data+size, "name\t%s\r\n", s_modinfo->info.name);
                size+=sprintf(data+size, "version\t%d.%d %d/%d/%d %d:%.2d\r\n", 
                    s_modinfo->info.entity.version.major, 
                    s_modinfo->info.entity.version.minor,
                    s_modinfo->info.entity.builddate.day,
                    s_modinfo->info.entity.builddate.month,
                    s_modinfo->info.entity.builddate.year,
                    s_modinfo->info.entity.builddate.hour,
                    s_modinfo->info.entity.builddate.min
                );
                size+=sprintf(data+size, "rom.addr\t0x%0.8X\r\n", s_modinfo->info.rom.addr);
                size+=sprintf(data+size, "rom.size\t0x%0.8X\r\n", s_modinfo->info.rom.size);
                size+=sprintf(data+size, "ram.addr\t0x%0.8X\r\n", s_modinfo->info.ram.addr);
                size+=sprintf(data+size, "ram.size\t0x%0.8X\r\n", s_modinfo->info.ram.size);

                size+=sprintf(data+size, "stg.type\t%s\r\n", (ee_strg_none == s_modinfo->info.storage.type) ? ("none") 
                                                                : ((ee_strg_eflash==s_modinfo->info.storage.type) ? ("flash") : ("eeprom")));
                size+=sprintf(data+size, "stg.addr\t0x%0.8X\r\n", s_modinfo->info.storage.addr);
                size+=sprintf(data+size, "stg.size\t0x%0.8X\r\n", s_modinfo->info.storage.size);
                size+=sprintf(data+size, "com.msk\t0x%0.8X\r\n\r\n", s_modinfo->info.communication);
            }

            *sizeout = size + 1;
          
            return 1;
        }
        */

        case CMD_BLINK:
        {
            for (uint8_t n=0; n<16; ++n)
            { 
                hal_led_toggle(hal_led2);
                osal_task_wait(250*1000);
            }

            return 0;
        }// break;

        case CMD_UPD_ONCE:
        {
            shalbase_ipc_gotoproc_set(ee_procUpdater);
            shalbase_system_restart();
            
            return 0;
        }// break;

        default:
        {
            hal_trace_puts("DEFAULT");
        } break;
    }

    return 0;
}


static eEresult_t s_sys_eeprom_erase(void)
{
#if 0    
   // ... it does not stay in memory    
   uint32_t i=0;
   static uint8_t data[EENV_STGSIZE-EENV_STGSTART] = {0};
   eEresult_t ret = ee_res_OK;
    
    // erase-eeprom
    hal_eeprom_erase(hal_eeprom_i2c_01, EENV_STGSTART, EENV_STGSIZE);  
    
    hal_eeprom_read(hal_eeprom_i2c_01, EENV_STGSTART, EENV_STGSIZE, data);

    for(i=0; i<EENV_STGSIZE-EENV_STGSTART; i++)
    {
        if(0xff != data[i])
        {
            ret = ee_res_NOK_generic;
        }
    } 

    return(ret);
#else    
    eEstorage_t strg = { .type = ee_strg_eeprom, .size = EENV_MEMMAP_SHALSYSTEM_STGSIZE, .addr = EENV_MEMMAP_SHALSYSTEM_STGADDR};
    return(shalbase_storage_clr(&strg, EENV_MEMMAP_SHALSYSTEM_STGSIZE));
#endif    
}

