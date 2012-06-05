/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
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


// --------------------------------------------------------------------------------------------------------------------
// - doxy
// --------------------------------------------------------------------------------------------------------------------

/* @file       EOMappTheSysController.c
    @brief      This file contains internal implementation of EOMappTheSysController singleton obj
    @author     valentina.gaggero@iit.it
    @date       01/02/2012
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------
#include "stdlib.h"
#include "string.h"

//abslayer
#include "hal.h" 
#include "hal_debugPin.h"

//embobj
#include "EOtheMemoryPool.h"
#include "EOtheErrorManager.h"
#include "EOtheActivator.h"
#include "EOMmutex.h"
#include "EOemsController.h"


//appl
#include "EOethBaseModule.h"
//#include "EOMappDataCollector.h"
//#include "EOMdatatransmitter.h"
//#include "EOMmotorController.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------
#include "EOMappTheSysController.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
#include "EOMappTheSysController_hid.h" 
#include "EOaction_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
#define EVT_CHECK(var, EVTMASK)             (EVTMASK == (var&EVTMASK))


#define        APP_THESYSCONTROL_TASKPRIO                65   

        
//************************ system controller task events **************************************************
#define         APP_THESYSCONTROL_EVT_DGRAMREC                  (1 << 0) 


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables. deprecated: better using _get(), _set() on static variables 
// --------------------------------------------------------------------------------------------------------------------
// empty-section

extern int16_t pwm_out;

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
void eom_appTheSysController_taskInit(void *p);
static void s_eom_appTheSysController_startup(EOMtask *p, uint32_t t);
static void s_eom_appTheSysController_run(EOMtask *tsk, uint32_t evtmsgper);

static void s_eom_appTheSysController_recDgram_mng(EOMappTheSysController *p);

static hal_result_t s_eom_appTheSysController_timers_init(void);
static void s_timer2_appFreq_cbk(void *p);
static void s_timer3_dataCollect_cbk(void *p);
static void s_timer4_motorCntrlStart_cbk(void *p);
static void s_eom_appTheSysController_timers_start(void);
static void s_eom_appTheSysController_timers_stop(void);

static eOresult_t s_eom_appTheSysController_confugureSystem(EOMappTheSysController *p);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
static const char s_eobj_ownname[] = "EOMappTheSysController";

static EOMappTheSysController s_theSysController;

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------
extern EOMappTheSysController* eom_appTheSysController_Initialise(EOMappTheSysController_cfg_t *cfg)
{
    hal_result_t res;
    EOMappTheSysController *retptr = NULL;

    if(NULL == cfg)
    {
        return(retptr);
    }

    retptr = &s_theSysController;

    //save in the singleton its configuration
    memcpy(&retptr->cfg, cfg, sizeof(EOMappTheSysController_cfg_t));

    retptr->mytask = NULL;

    //initiatilse its timers
    res = s_eom_appTheSysController_timers_init();
    if(hal_res_OK != res)
    {
        return(retptr);
    }
    //initialize its eth module: use to communicate with pc104 by eth.
    retptr->eth_mod = eo_ethBaseModule_New(&(cfg->ethmod_cfg));

    // change its status
    retptr->st = eOm_appTheSysController_st__inited;
    
    return(retptr);    
}



extern EOMappTheSysController* eom_appTheSysController_GetHandle(void)
{
    return(&s_theSysController);
}

extern eOresult_t eom_appTheSysController_Start(EOMappTheSysController *p)
{

    p->mytask = eom_task_New(eom_mtask_EventDriven,
                                      APP_THESYSCONTROL_TASKPRIO,
                                      3*1024, //stacksize: da rivedere
                                      s_eom_appTheSysController_startup, 
                                      s_eom_appTheSysController_run,  
                                      0, //message queue size. the task is evt based
                                      eok_reltimeINFINITE,
                                      p, 
                                      eom_appTheSysController_taskInit, 
                                      "sysController");
    return(eores_OK);
}

//this function is used only in this file, but it must not defined static for debug task viewer.
void eom_appTheSysController_taskInit(void *p)
{
    eom_task_Start(p);
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------
/*
    3 timers are employed by the SysContyroller:
        - timer 2: period = 1000 us. (application_frequency). The application must finish its task in 1000 us. 
        - timer 3: period = 500 us.  (data_collect)time for data collection(from eth, can and encoders))the data collector must finish before this timeout 
        - timer 4: period = 600 us   (motor control start)
*/

static hal_result_t s_eom_appTheSysController_timers_init(void)
{
    hal_result_t res;

//timer1 is not supported by hal.
    hal_timer_cfg_t t2_appFreq_cfg = 
    {
        .prescaler          = hal_timer_prescalerAUTO,         
        .countdown          = 1000,
        .priority           = hal_int_priority02,
        .mode               = hal_timer_mode_periodic,
        .callback_on_exp    = s_timer2_appFreq_cbk,
        .arg                = NULL
    };

    hal_timer_cfg_t t3_dataCollect_cfg = 
    {
        .prescaler          = hal_timer_prescalerAUTO,         
        .countdown          = 500,
        .priority           = hal_int_priority02,
        .mode               = hal_timer_mode_oneshot,
        .callback_on_exp    = s_timer3_dataCollect_cbk,
        .arg                = NULL
    };

    hal_timer_cfg_t t4_motorCntrlStart_cfg = 
    {
        .prescaler          = hal_timer_prescalerAUTO,         
        .countdown          = 600,
        .priority           = hal_int_priority02,
        .mode               = hal_timer_mode_oneshot,
        .callback_on_exp    = s_timer4_motorCntrlStart_cbk,
        .arg                = NULL
    };

    res = hal_timer_init(hal_timer2, &t2_appFreq_cfg, NULL);
    if(hal_res_OK != res)
    {
        return(res);
    }
    hal_timer_init(hal_timer3, &t3_dataCollect_cfg, NULL);
    if(hal_res_OK != res)
    {
        return(res);
    }
    hal_timer_init(hal_timer4, &t4_motorCntrlStart_cfg, NULL);
    if(hal_res_OK != res)
    {
        return(res);
    }
    return(hal_res_OK);
}

static void s_timer2_appFreq_cbk(void *p)
{
        hal_led_on(hal_led0);
        hal_led_off(hal_led1);

        DEBUG_PIN_ON;
        if(NULL != s_theSysController.cfg.sig2appDataCollector_start.fn)
        {
            s_theSysController.cfg.sig2appDataCollector_start.fn(s_theSysController.cfg.sig2appDataCollector_start.argoffn);
        }

//        eom_appDataCollector_CollectDataStart(p->appDataCollector_ptr);
        hal_timer_start(hal_timer3);
        hal_timer_start(hal_timer4);

}


static void s_timer3_dataCollect_cbk(void *p)
{
    hal_led_on(hal_led2);
    if(NULL != s_theSysController.cfg.sig2appDataCollector_stop.fn)
    {
        s_theSysController.cfg.sig2appDataCollector_stop.fn(s_theSysController.cfg.sig2appDataCollector_stop.argoffn);
    }
//        eom_appDataCollector_CollectDataStop(p->appDataCollector_ptr);
}


static void s_timer4_motorCntrlStart_cbk(void *p)
{

    hal_led_on(hal_led2);
    if(NULL != s_theSysController.cfg.sig2appMotorController.fn)
    {
        s_theSysController.cfg.sig2appMotorController.fn(s_theSysController.cfg.sig2appMotorController.argoffn);
    }

    
//    eom_motorController_calcStart(motorController_objPtr);

}
static void s_eom_appTheSysController_timers_start(void)
{
    hal_timer_start(hal_timer2);
}


static void s_eom_appTheSysController_timers_stop(void)
{
    hal_timer_stop(hal_timer2);
}


static void s_eom_appTheSysController_startup(EOMtask *tsk, uint32_t t)
{
    EOMappTheSysController *p = (EOMappTheSysController*)eom_task_GetExternalData(tsk);
    EOaction a;

    eo_errman_Assert(eo_errman_GetHandle(), NULL != p, s_eobj_ownname, "extdata() is NULL");

    //configure the action to perform on receive datagram
    eo_action_SetEvent(&a, (eOevent_t)APP_THESYSCONTROL_EVT_DGRAMREC , tsk);
    eo_ethBaseModule_SetActionOnRec(p->eth_mod, &a);

    //start the ethernet module: after this I can receive datagram
    eo_ethBaseModule_Activate(p->eth_mod);

    hal_led_off(hal_led0);
    hal_led_off(hal_led1);
} 


static void s_eom_appTheSysController_run(EOMtask *tsk, uint32_t evtmsgper)
{
    eOevent_t evt;

    EOMappTheSysController *p = (EOMappTheSysController*)eom_task_GetExternalData(tsk);

    evt = (eOevent_t)evtmsgper;


    if(EVT_CHECK(evt, APP_THESYSCONTROL_EVT_DGRAMREC))
    {
        s_eom_appTheSysController_recDgram_mng(p); //received dtagagram manager
    }

}

#define MC_START       1
#define MC_STOP        2
#define MC_SET_POS     3
#define MC_SET_POS_PID 4
#define MC_SET_POS_LIM 5
#define MC_SET_VEL     6
#define MC_SET_VEL_PID 7
#define MC_SET_VEL_LIM 8
#define MC_SET_OFFSET  9

static void s_eom_appTheSysController_recDgram_mng(EOMappTheSysController *p)
{
    uint8_t *payload_ptr;
    uint16_t payloadsize;
    //eOresult_t res = eores_OK;

    //p and &s_theSysController are equal. 
    eo_ethBaseModule_Receive(p->eth_mod, &payload_ptr, &payloadsize);

    // ALE

    switch(payload_ptr[0]) 
    {
        case MC_START:  //Corrisponde a PKT QUERY in WINNODE utils appl
        {
            if(eOm_appTheSysController_st__inited == p->st)
            {
                p->st = eOm_appTheSysController_st__configured;
                s_eom_appTheSysController_confugureSystem(p);
            }
                 
            p->st = eOm_appTheSysController_st__started;
            s_eom_appTheSysController_timers_start();
        }
        break;

        case MC_STOP: //Corrisponde a ADDCLNT in WINNODE utils appl
        {
            if(eOm_appTheSysController_st__started == p->st)
            {
                p->st = eOm_appTheSysController_st__stopped;
                s_eom_appTheSysController_timers_stop();
                
                eo_emsController_Stop(0);
                eo_emsController_ResetPosPid(0);
                eo_emsController_ResetVelPid(0);
                pwm_out = 0;
            }
            
        }
        break;

        case MC_SET_POS:
        {
            uint8_t joint = payload_ptr[1];
            int32_t pos = *(int32_t*)(payload_ptr+4);
            int32_t vel = *(int32_t*)(payload_ptr+8);

            eo_emsController_SetPosRef(joint, pos, vel);
        }
        break;

        case MC_SET_POS_PID:
        {
            uint8_t joint = payload_ptr[1];
            int32_t kp = *(int32_t*)(payload_ptr+4);
            int32_t kd = *(int32_t*)(payload_ptr+8);
            int32_t ki = *(int32_t*)(payload_ptr+12);
            uint8_t shift = *(uint8_t*)(payload_ptr+16);

            eo_emsController_SetPosPid(joint, kp, kd, ki, shift);
        }
        break;

        case MC_SET_POS_LIM:
        {
            uint8_t joint = payload_ptr[1];
            int16_t Ymax = *(int16_t*)(payload_ptr+4);
            int16_t Imax = *(int16_t*)(payload_ptr+8);

            eo_emsController_SetPosPidLimits(joint, Ymax, Imax);
        }
        break;

        case MC_SET_VEL:
        {
            uint8_t joint = payload_ptr[1];
            int32_t vel = *(int32_t*)(payload_ptr+4);
            int32_t acc = *(int32_t*)(payload_ptr+8);

            eo_emsController_SetVelRef(joint, vel, acc);
        }
        break;

        case MC_SET_VEL_PID:
        {
            uint8_t joint = payload_ptr[1];
            int32_t kp = *(int32_t*)(payload_ptr+4);
            int32_t kd = *(int32_t*)(payload_ptr+8);
            int32_t ki = *(int32_t*)(payload_ptr+12);
            uint8_t shift = *(uint8_t*)(payload_ptr+16);

            eo_emsController_SetVelPid(joint, kp, kd, ki, shift);
        }
        break;

        case MC_SET_VEL_LIM:
        {
            uint8_t joint = payload_ptr[1];
            int16_t Ymax = *(int16_t*)(payload_ptr+4);
            int16_t Imax = *(int16_t*)(payload_ptr+8);

            eo_emsController_SetVelPidLimits(joint, Ymax, Imax);
        }
        break;

        case MC_SET_OFFSET:
        {
            uint8_t joint = payload_ptr[1];
            int16_t off = *(int16_t*)(payload_ptr+4);
            
            eo_emsController_SetOffset(joint, off);
        }
        break;

    }

    /*
    if(eores_OK == res)
    {
        eo_ethBaseModule_Transmit(p->eth_mod, payload_ptr, payloadsize);    
    }
    */
}
static eOresult_t s_eom_appTheSysController_confugureSystem(EOMappTheSysController *p)
{
    eOmc_motorId_t mId = 0;
    eOmc_motor_config_t cfg =
    { 
        EO_INIT(.pidcurrent)
        {
            EO_INIT(.kp)                    0x1111,
            EO_INIT(.ki)                    0x2222,
            EO_INIT(.kd)                    0x3333,
            EO_INIT(.limitonintegral)       0X4444,
            EO_INIT(.limitonoutput)         0x5555,
            EO_INIT(.scale)                 0x0,
            EO_INIT(.offset)                0x6666,
            EO_INIT(.filler03)              {0xf1, 0xf2, 0xf3}
        },
        EO_INIT(.maxvelocityofmotor)        0xAA,
        EO_INIT(.maxcurrentofmotor)         0xBB,
        EO_INIT(.des02FORmstatuschamaleon04)   {0}
    };

#warning X ALE --> set static motor config  in cfg var

    return(eo_appCanSP_ConfigMotor(p->cfg.canSP_ptr, mId, &cfg));
}
// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



