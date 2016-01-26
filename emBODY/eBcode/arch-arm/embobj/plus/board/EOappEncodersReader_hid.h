/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero, Davide Pollarolo
 * email:   valentina.gaggero@iit.it, davide.pollarolo@iit.it
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

#ifndef _EOAPPENCODERSREADER_HID_H_
#define _EOAPPENCODERSREADER_HID_H_


// - doxy -------------------------------------------------------------------------------------------------------------
/* @file       EOappEncodersReader.h
    @brief     This file provides hidden interface to encodersReader obj.
    @author    valentina.gaggero@iit.it, davide.pollarolo@iit.it
    @date       02/17/2012
**/


// - external dependencies --------------------------------------------------------------------------------------------


#include "hal_spiencoder.h"


// - declaration of extern public interface ---------------------------------------------------------------------------

#include "EOappEncodersReader.h"


// - definition of the hidden struct implementing the object ----------------------------------------------------------


/** @typedef    struct eOappEncReader_stream_t
    @brief      contains representation of an SPI stream of hal encoders. They all must be of the same type.
 **/
typedef struct 
{
    hal_spiencoder_type_t      type;                               /**< the type of SPI encoders. They must be all homogeneous */
    uint8_t                 numberof;                           /**< their number inside encoders[] */
} eOappEncReader_stream_t;

//typedef enum
//{
//    eo_appEncReader_stream0     = 0,  /* SPI stream 0 */ 
//    eo_appEncReader_stream1     = 1,  /* SPI stream 1 */
//    eo_appEncReader_streamNONE  = 255 /* SPI stream NOT DEFINED */     
//} eo_appEncReader_stream_number_t;

//enum { eo_appEncReader_streams_numberof = hal_spiencoder_streams_number };

//typedef enum
//{
//    eo_appEncReader_stream_position0    = 0,  
//    eo_appEncReader_stream_position1    = 1,  
//    eo_appEncReader_stream_position2    = 2,
//    eo_appEncReader_stream_positionNONE = 255,    
//} eo_appEncReader_stream_position_t;

//enum { eo_appEncReader_stream_position_numberof = hal_spiencoder_maxnumber_in_stream };


//typedef enum
//{
//    hal_spiencoder_spistream0      = 0,
//    hal_spiencoder_spistream1      = 1,
//    hal_spiencoder_spistreamNONE   = 255   
//} hal_spiencoder_spistream_t; 



//typedef struct
//{
//    uint8_t     numberofspiencoders;
//    uint8_t     numberofspiencodersStream0;
//    uint8_t     numberofspiencodersStream1;
//    uint8_t     maskofsupported[hal_spiencoders_number];
//    uint8_t     usedstream[hal_spiencoders_number];
//    uint8_t     indexinstream[hal_spiencoders_number];
//} hal_spiencoder_spimapping_t;

// the application is able to read until a maximum of 6 encoders (EMS board): 3 connected with one SPI (SPI2) and others 3 connected with another SPI bus (SPI3).
//

typedef struct
{
    hal_spiencoder_t first;                                      /**< indicates the first encoder to read */
    hal_spiencoder_t list[hal_spiencoder_maxnumber_in_stream+1];   /**< for the encoder i-th: in list[i] the encoder i-th finds the number of encoder it must start when it has finish its onw read. */
} EOappEncReader_configEncSPIXReadSequence_hid_t;


typedef enum
{
    eOEncReader_readSt__idle      = 0,
    eOEncReader_readSt__started   = 1,
    eOEncReader_readSt__finished  = 2  
} eOappEncReader_readStatusSPIX_t;


typedef struct
{
    eOappEncReader_readStatusSPIX_t                 st;                  /**< contains the status of reading on SPIX (1 or 3) */
    eo_appEncReader_encoder_type_t                  enc_type;            /**< the type of the encoder to be read (AEA and AMO supported) */
    uint8_t                                         enc_numbers;         //number of encoders associated
    uint8_t                                         enc_number_supported;
    EOappEncReader_configEncSPIXReadSequence_hid_t  readSeq;             /**< contains the sequence of reading of encoders connected to a stream */ 
} EOappEncReader_confEncDataPerSPI_hid_t;


struct EOappEncReader_hid
{
    eObool_t                                initted;
    eObool_t                                active;
    const hal_spiencoder_stream_map_t*         stream_map;
    eOappEncReader_cfg_t                    config;       
    eOappEncReader_stream_t                 SPI_streams[hal_spiencoder_streams_number];  // SPI streams; must be coherent with what inside cfg
    EOappEncReader_confEncDataPerSPI_hid_t  configuredEnc_SPI_stream0;      /* Encoders configured on the first  SPI stream */
    EOappEncReader_confEncDataPerSPI_hid_t  configuredEnc_SPI_stream1;      /* Encoders configured on the second SPI stream */
    uint64_t                                times[2][4];
}; 


// - declaration of extern hidden functions ---------------------------------------------------------------------------
// empty-section


#endif  // include guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



