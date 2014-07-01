/*
 * firmware/controller application.
 *
 */

#include "dsp56f807.h"
#include "options.h"
#include "pid.h"
#include "controller.h"
#include "trajectory.h"
#include "abs_ssi_interface.h"
#include "can1.h"
#include "can_interface.h"
#include "pwm_interface.h"

#include "encoders_interface.h"
#include "calibration.h"
#include "control_enable.h"

#ifndef VERSION
#	error "No valid version specified"
#endif

Int16 _max_position_enc_tmp[JN] = INIT_ARRAY (0);

//helper functions
void helper_calib_hard_stops(byte channel, Int16 param1,Int16 param2, Int16 param3);
void helper_calib_abs_digital(byte channel, Int16 param1,Int16 param2, Int16 param3);
void helper_calib_abs_and_incremental(byte channel, Int16 param1,Int16 param2, Int16 param3);

/************************************************************ 
 * this function checks if the calibration is terminated
 * and if calibration is terminated resets the encoder
 ************************************************************/
void check_in_position_calib(byte jnt)
{
	Int32 temporary_long;
	bool temporary_cond1;
	bool temporary_cond2; 
	bool temporary_cond3; 
	
	// increase the counter for the calibration (wait for the movement to start)
	_counter_calib +=1;
	// final consideration reached? and ... 
	temporary_long = (Int32) extract_h(_filt_abs_pos[jnt]);
	temporary_cond1 = (__abs( temporary_long - _abs_pos_calibration[jnt]) < INPOSITION_CALIB_THRESHOLD);
	temporary_cond2 = (_position[jnt] == _position_old[jnt]);
	// ... control mode is calibration? and ...
	temporary_cond1 = temporary_cond1 && (_control_mode[jnt] == MODE_CALIB_ABS_POS_SENS);
	temporary_cond2 = temporary_cond2 && (_control_mode[jnt] == MODE_CALIB_HARD_STOPS);
	temporary_cond3 =                    (_control_mode[jnt] == MODE_CALIB_ABS_AND_INCREMENTAL);
	// ... trajecotry ended? 
	temporary_cond1 = temporary_cond1 && _ended[jnt];
	temporary_cond2 = temporary_cond2 && (_counter_calib > 1000);
	temporary_cond3 = temporary_cond3 && _ended[jnt];
		
	if (temporary_cond1 | temporary_cond2 | temporary_cond3)
	{
		
#if VERSION != 0x0119
		_control_mode[jnt] = MODE_POSITION;
		set_position_encoder (jnt, 0);
#endif

#if ((VERSION == 0x0128) || (VERSION == 0x0228))
		if (jnt!=0)
		{
			_max_position_enc[jnt] = _max_position_enc_tmp[jnt];					
			_min_position_enc[1]=-1800; //Thumb proximal right and left
			
			if (_max_position_enc[2]>0)
				_min_position_enc[2]=-3000; //Thumb distal
   			 else 
   			 	_min_position_enc[2]=3000; //Thumb distal
  
			_min_position_enc[3]=-300; //Index proximal  right and left
			
			_calibrated[jnt] = true;
			return;
		}		

#elif ((VERSION == 0x0130) || (VERSION == 0x0230))
		{
			_max_position_enc[jnt] = _max_position_enc_tmp[jnt];
		
			if (_max_position_enc[0]>0)
				_min_position_enc[0]=-3000; //Index distal
			else
				_min_position_enc[0]=3000; //Index distal
				
			_min_position_enc[1]=-300; //Middle proximal right and left
			
			if (_max_position_enc[2]>0)
				_min_position_enc[2]=-3000; //Middle distal
			else
				_min_position_enc[2]=3000; //Middle distal	
			
			_min_position_enc[3]=0; //little fingers  right and left
				
			_calibrated[jnt] = true;
			return;
		}						
		
#elif VERSION ==0x0119

		_control_mode[jnt] = MODE_POSITION;	

		if (jnt == 0)
		{
			set_position_encoder (jnt, 0);
		}
		if (jnt==1)
		{
			set_position_encoder (2, _position[2]);
			set_position_encoder (1, 0);
			_position[2] = _position[1] + _position[2];
		}
		if (jnt==2)
		{
			set_position_encoder (2, -_position[1]);		
			_position[2] = _position[1] + _position[2];		
		}	
#endif
		_position[jnt] = 0;
		_position_old[jnt] = 0;
		_integral[jnt] = 0;
		//Keep the system in the current configuration
		_set_point[jnt] = _position[jnt];
		init_trajectory (jnt, _position[jnt], _position[jnt], 1);
		_calibrated[jnt] = true;
	}
}

/**************************************************************** 
 * helper functions
 ****************************************************************/
void helper_calib_hard_stops(byte channel, Int16 param1,Int16 param2, Int16 param3)
{
	if (!mode_is_idle(channel) && IS_DONE(channel))
	{
		_control_mode[channel] = MODE_CALIB_HARD_STOPS;	
		_counter_calib = 0;
		_pwm_calibration[channel] = param1;
		if (param2!=0)
	 		_velocity_calibration[channel]=param2;
		else
			_velocity_calibration[channel]=1;
	}		
}

void helper_calib_abs_digital(byte channel, Int16 param1,Int16 param2, Int16 param3)
{
	if (param3 >=0 && param3 <=4095) 
	{
		set_max_position(channel, param3);	
		#if (VERSION==0x0250 || VERSION==0x0251 || VERSION==0x0252 || VERSION==0x0254 || VERSION==0x0257 ) 
		set_position_encoder(channel,0);
		#endif
	}
	if (param2>0)
	{
	    _position[channel] = get_position_abs_ssi(channel);
		_set_point[channel] = param1;
		init_trajectory (channel, _position[channel], _set_point[channel], param2);
		_calibrated[channel] = true;
	}
	if (param2==0)
	{
		put_motor_in_fault(channel);
		can_printf ("invalid calib p2");				
	}	
}

void helper_calib_abs_and_incremental(byte channel, Int16 param1,Int16 param2, Int16 param3)
{
	if (param2>0)
	{
	    _control_mode[channel] = MODE_CALIB_ABS_AND_INCREMENTAL;
	    _set_point[channel] = param1;
		_max_position_enc_tmp[channel] = param3;

		init_trajectory(channel, _position[channel], _set_point[channel], param2);
	}
	if (param2==0)
	{
		put_motor_in_fault(channel);		
		can_printf ("invalid calib p2");
	} 			
}

/**************************************************************** 
 * calibration procedure, depends on the firmware version.
 ****************************************************************/
void calibrate (byte channel, byte type, Int16 param1,Int16 param2, Int16 param3)
{
//-------------------------------
// 1.11 4DC
//-------------------------------
#if VERSION == 0x0111
	 
	if (type==CALIB_HARD_STOPS)	helper_calib_hard_stops (channel, param1, param2,param3);

//-------------------------------
// 1.15 4DC
//-------------------------------
#elif VERSION ==0x0115
	 
	if (type==CALIB_HARD_STOPS) helper_calib_hard_stops (channel, param1, param2,param3);

//-------------------------------
// 2.14 4DC
//-------------------------------
#elif VERSION ==0x0214

	if ((type==CALIB_HARD_STOPS) && (channel<=2)) helper_calib_hard_stops (channel, param1, param2,param3);


	if ((type==CALIB_ABS_DIGITAL ) && (channel==3))
	{		
		if ((param3 >=0 && param3 <=4095) )	set_max_position(AEA4, param3);	
		
		if (param2>0)
		{		
		    _position[channel] = get_position_abs_ssi(AEA3);
			_set_point[channel] = param1;
			init_trajectory (channel, _position[channel], _set_point[channel], param2);
			_calibrated[channel] = true;
		}
		if (param2==0)
		{
			put_motor_in_fault(channel);
			can_printf ("invalid calib p2");		
		}		
	}	
//-------------------------------
// 2.15 4DC
//-------------------------------	
#elif VERSION ==0x0215

	if ((type==CALIB_HARD_STOPS) && (channel>=2)) helper_calib_hard_stops (channel, param1, param2,param3);

	if ((type==CALIB_ABS_DIGITAL ) && (channel<=1))
	{	
		if ((param3 >=0 && param3 <=4095) && (channel==0))
			set_max_position(AEA3, param3);	
		else
			set_max_position(AEA4, param3);

		if (param2>0)
		{
		    _position[channel] = ((channel==0) ? get_position_abs_ssi(AEA3): get_position_abs_ssi(AEA4));
			_set_point[channel] = param1;
			init_trajectory (channel, _position[channel], _set_point[channel], param2);
			_calibrated[channel] = true;
		}
		if (param2==0)
		{
			put_motor_in_fault(channel);
			can_printf ("invalid calib p2");		
		}		
	}	

//-------------------------------	 
// 1.19 2.19 4DC   
//-------------------------------	 
#elif (VERSION ==0x0219 || VERSION == 0x0119)

	//pronosupination J4
	if ((type==CALIB_HARD_STOPS) && (channel==0)) helper_calib_hard_stops (channel, param1, param2,param3);

	//wrist J5 J6
	if (type==CALIB_HARD_STOPS_DIFF) //VERSION 0x119 ONLY
	{	
		byte channel1;
	    byte channel2; 
		if (channel==1)
		{
			channel1=1; //maybe it could be change somehow 
			channel2=2;
		}
		else
		{
			channel1=2; //maybe it could be change somehow 
			channel2=1;
		}
		if ((!mode_is_idle(channel1) && IS_DONE(channel1)) && 
		    (!mode_is_idle(channel2) && IS_DONE(channel2)))
		{
			_control_mode[channel1] = MODE_CALIB_HARD_STOPS;
			_counter_calib = 0;
			_pwm_calibration[channel1] = param1;
			_pwm_calibration[channel2] = param1;
			if (param2!=0)
		 		_velocity_calibration[channel1]=param2;
			else
				_velocity_calibration[channel1]=1;
		}	
	}	

	if (type==CALIB_ABS_DIGITAL )  //VERSION 0x129 ONLY
	{
		//wrist J5 J6	
		if (channel!=3)
		{					
			if (channel==1)
			{
				if (param3 >=0 && param3 <=4095) set_max_position(AEA6, param3);	
				_position[channel] = get_position_abs_ssi(AEA6);
			}
			if (channel==2)
			{
				if (param3 >=0 && param3 <=4095) set_max_position(AEA5, param3);	
				_position[channel] = get_position_abs_ssi(AEA5);
			}
		
			if (param2>0)
			{
				_calibrated[channel] = true;
				if (_calibrated[1] == true && 
				    _calibrated[2] == true )
				{
					enable_motor_pwm(channel);
				}
				    		    
				_set_point[channel] = param1;
				init_trajectory (channel, _position[channel], _set_point[channel], param2);
			}
			else
			{
				put_motor_in_fault(channel);		
				can_printf ("invalid calib p2");	
			}	
		}
		// FINGER J7
		else
		{
			if (param2>0)
			{
				_calibrated[channel] = true;
			    enable_motor_pwm(channel);

				_set_point[channel] = param1;
				init_trajectory(channel, _position[channel], _set_point[channel], param2);		
			}
			else
			{
			   put_motor_in_fault(channel);		
			   can_printf ("invalid calib p2");				
			} 		
			
		}
	}

//-------------------------------	 	  
// 1.28  2.28 4DC	 
//-------------------------------	 
 
#elif ((VERSION == 0x0128) ||(VERSION == 0x0228) )

   //FINGER J8
	if ((type==CALIB_ABS_DIGITAL) && (channel==0) ) 
	{
		if (param2>0)
		{
			_calibrated[channel] = true;
			enable_motor_pwm(channel);
				
			_set_point[channel] = param1;
			init_trajectory(channel, _position[channel], _set_point[channel], param2);
		}
		else
		{
			put_motor_in_fault(channel);		
			can_printf ("invalid calib p2");	
		} 			
	}	

    //FINGER J9 J10 J11
	if ((type==CALIB_ABS_AND_INCREMENTAL) &&  (channel!=0)) helper_calib_abs_and_incremental (channel, param1, param2,param3);	
	
//-------------------------------	 	  
//  1.30 2.30   4DC  	 
//-------------------------------	 
 
#elif ((VERSION == 0x0130) || (VERSION == 0x0230))

    //FINGER J12 J13 J14 J15
	if ((type==CALIB_ABS_AND_INCREMENTAL) ) helper_calib_abs_and_incremental (channel, param1, param2,param3);	

//-------------------------------	 	  
// 3.51     2BLLIE  
//-------------------------------	 
#elif (VERSION==0x0351)

	//iKart
	if (type==CALIB_ABS_DIGITAL) 
	{
		set_current_as_middle_position(channel);
		set_relative_position_abs_ssi_turns(channel, 0);
		_position[channel] = get_relative_position_abs_ssi(channel);				
	}

//-----------------------------------	  
// 2.50 2.51 2.52 2.54 2.57 2BLLIE 	 
//-----------------------------------	 
#elif (VERSION==0x0250 || VERSION==0x0251 || VERSION==0x0252 || VERSION==0x0254 || VERSION==0x0257 ) 

	// standard bllIe calibrator
	if (type==CALIB_ABS_DIGITAL)  helper_calib_abs_digital (channel, param1, param2,param3);

//-----------------------------------	  
// 1.61 2BLLDC 	 
//-----------------------------------
#elif VERSION==0x0161

	if (type==CALIB_HARD_STOPS) helper_calib_hard_stops (channel, param1, param2,param3);
			
//-----------------------------------	  
// 1.62 2BLLDC 	 
//-----------------------------------
#elif VERSION==0x0162

	//neck V2
	if (type==CALIB_ABS_DIGITAL)  helper_calib_abs_digital (channel, param1, param2,param3);

//-----------------------------------	  
// 1.40 1.50 1.51 1.52 1.54 2BLL 	 
//-----------------------------------	 
#elif (VERSION==0x0140 || VERSION==0x0150 || VERSION==0x0151 || VERSION==0x0152 || VERSION==0x0154 ) 

	//first two joints of the shoulder
    if (type==CALIB_ABS_DIGITAL)  helper_calib_abs_digital (channel, param1, param2,param3);

//-----------------------------------	  
// 1.47 1.57 2BLL 	 
//-----------------------------------
#elif  VERSION==0x0157 || VERSION==0x0147 

	//special calibrator for coupled shoulder joint (V1 mechanics)
	if (param3 >=0 && param3 <=4095)
	{
		set_max_position(channel, param3);	
	}
	if (param2>0 && channel==0)
	{
		set_relative_position_abs_ssi_turns(channel, 0);
		_position[channel] = get_relative_position_abs_ssi(channel);
		#if VERSION==0x0157 
			_position[channel] = (((float) _position[channel])*0.6153F);  
			_position[channel] = _position[channel]+ _cpl_pos_prediction[0];
			_position[channel] = _position[channel]- _cpl_pos_prediction[1];			
		#endif			
		_set_point[channel] = param1;
		init_trajectory (channel, _position[channel], _set_point[channel], param2);
		_calibrated[channel] = true;
	}
	if (param2>0 &&channel==1)
	{
	    _position[channel] = get_position_abs_ssi(channel);
		_set_point[channel] = param1;
		init_trajectory (channel, _position[channel], _set_point[channel], param2);
		_calibrated[channel] = true;
	}
	if (param2==0)
	{
		put_motor_in_fault(channel);
		can_printf ("invalid calib p2");				
	}
#else

	//this should be unreachable
	#warning UNDEFINED CALIBRATOR TYPE!!
	#error   UNDEFINED CALIBRATOR TYPE!!
	
#endif

 return;	
}