/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
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

// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _EOCFG_NVSEP_MN_HID_H_
#define _EOCFG_NVSEP_MN_HID_H_

#ifdef __cplusplus
extern "C" {
#endif


/** @file       eOcfg_nvsEP_mn_hid.h
	@brief      This header file gives the ...
	@author     marco.accame@iit.it
	@date       09/06/2011
**/

/** @defgroup eo_h2uilsdede2345er Configuration of the bodypart
    In here there are constants for the max number of strains and maiss in an endpoint 
    
    @{		
 **/



// - external dependencies --------------------------------------------------------------------------------------------

#include "EoCommon.h"
#include "EOnv.h"

#include "eOcfg_nvsEP_mn.h"


// - public #define  --------------------------------------------------------------------------------------------------
// empty-section


// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------


// - declaration of extern public functions ---------------------------------------------------------------------------


// - comm

extern void eo_cfg_nvsEP_mn_comm_hid_INITIALISE(eOnvEP_t ep, void *loc, void *rem);

// init:    n is not used
extern void eo_cfg_nvsEP_mn_comm_hid_INIT__ropsigcfgcommand(uint16_t n, const EOnv* nv);


// updt:    n is not used
extern void eo_cfg_nvsEP_mn_comm_hid_UPDT__ropsigcfgcommand(uint16_t n, const EOnv* nv, const eOabstime_t time, const uint32_t sign);


// - appl

extern void eo_cfg_nvsEP_mn_appl_hid_INITIALISE(eOnvEP_t ep, void *loc, void *rem);

// init:    n is not used
extern void eo_cfg_nvsEP_mn_appl_hid_INIT_config(uint16_t n, const EOnv* nv);
extern void eo_cfg_nvsEP_mn_appl_hid_INIT_status(uint16_t n, const EOnv* nv);
extern void eo_cfg_nvsEP_mn_appl_hid_INIT_cmmnds__go2state(uint16_t n, const EOnv* nv);

// updt:    n is not used
extern void eo_cfg_nvsEP_mn_appl_hid_UPDT_config(uint16_t n, const EOnv* nv, const eOabstime_t time, const uint32_t sign);
extern void eo_cfg_nvsEP_mn_appl_hid_UPDT_status(uint16_t n, const EOnv* nv, const eOabstime_t time, const uint32_t sign);
extern void eo_cfg_nvsEP_mn_appl_hid_UPDT_cmmnds__go2state(uint16_t n, const EOnv* nv, const eOabstime_t time, const uint32_t sign);

/** @}            
    end of group eo_h2uilsdede2345er  
 **/

#ifdef __cplusplus
}       // closing brace for extern "C"
#endif 
 
#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


