#include "dsp56f807.h"
#include "options.h"
#include "pid.h"
#include "asc.h"
#include "can1.h"
#include "trajectory.h"
#include "serial_interface.h"
#include "pwm_interface.h"
#include "flash_interface.h"
#include "can_interface.h"
#include "eeprom_interface.h"

#ifndef VERSION
#	error "No valid version specified"
#endif

//const char p[]="pippo";


/***********************************************************
 global variables
************************************************************/

extern  char    _additional_info[32];
extern byte	_board_ID;
byte   serial_channels = 0;

/***************************************************************************/
/**
 * This method inits the serial interface
 * @param channels_num is the max number of channels on the board (2,4) 
 ***************************************************************************/
void serial_interface_init (byte channels_num)
{
	AS1_Init ();
	serial_channels = channels_num;
}
	
/***************************************************************************/
/**
 * prints the version number of the firmware to the serial port.
/***************************************************************************/
void print_version(void)
{
	AS1_printStringEx ("\r\n\n");
	AS1_printStringEx ("Firmware - ver ");
#if VERSION == 0x0111
	AS1_printStringEx ("1.11");
#elif VERSION == 0x0112
	AS1_printStringEx ("1.12");
#elif VERSION == 0x0113
	AS1_printStringEx ("1.13");
#elif VERSION == 0x0114
	AS1_printStringEx ("1.14");
#elif VERSION == 0x0115
	AS1_printStringEx ("1.15");
#elif VERSION == 0x0116
	AS1_printStringEx ("1.16");
#elif VERSION == 0x0117
	AS1_printStringEx ("1.17");
#elif VERSION == 0x0118
	AS1_printStringEx ("1.18");
#elif VERSION == 0x0119
	AS1_printStringEx ("1.19");
#elif VERSION == 0x0120
	AS1_printStringEx ("1.20");	
#elif VERSION == 0x0125
	AS1_printStringEx ("1.25");
#endif
	AS1_printStringEx ("\r\n");
}

/***************************************************************************/
/**
 * This method reads and executes the serial messages.
/***************************************************************************/
void serial_interface (void)
{
#ifdef SERIAL_MENU_OPTION

	byte c = 0;
	static byte axis = 0;
	
	UInt32 filter1=0;
	UInt32 filter2=0;
	UInt32 mask1=0;
	UInt32 mask2=0;
	
byte rd_eeprom[1];
	
#ifdef DEBUG_SERIAL
	byte tx, rx;
#endif
	Int32 acceptance_code;
	byte d = 0;
	char buffer[SMALL_BUFFER_SIZE];
	int  iretval = 0;
	char string_tmp [20];
	
	if (c == 0)
		AS1_RecvChar(&c);
	
	switch (c)
	{
		default:
			c = 0;
			break;
		
		case 'h':
		case 'H':
		case '\r':
			print_version ();
			AS1_printStringEx ("h, H: help\r\n");
			AS1_printStringEx ("a, set card address\r\n");
			AS1_printStringEx ("b, print card address\r\n");
			AS1_printStringEx ("A, set additional info\r\n");
			AS1_printStringEx ("B, print additional info\r\n");
			AS1_printStringEx ("w, write control params to FLASH mem\r\n");
			AS1_printStringEx ("r, read control params from FLASH mem\r\n");
			AS1_printStringEx ("v, toggle verbose flag\r\n");
			AS1_printStringEx ("n, print fault registers\r\n");

#ifdef DEBUG_SERIAL
			AS1_printStringEx ("d, dump flash memory\r\n");				
			AS1_printStringEx ("f, print CAN bus error values\r\n");
			AS1_printStringEx ("e, enable controller channel\r\n");
			AS1_printStringEx ("g, set pid gain\r\n");
			AS1_printStringEx ("s, show pid gain\r\n");
			AS1_printStringEx ("x1, start trajectory generation\r\n");
			AS1_printStringEx ("x2, stop trajectory generation\r\n");
			AS1_printStringEx ("x3, enable/disable PWM\r\n");
			AS1_printStringEx ("c, toggle channel 0/1\r\n");
#endif

			c = 0;
			break;

#ifdef DEBUG_SERIAL

		case 'd':
			dump_flash_memory();
			c = 0;
			break;
			
		case 'f':
			CAN1_getErrorValues (&rx, &tx);
			AS1_printStringEx ("rx = ");
			AS1_printByteAsChars (rx);
			AS1_printStringEx ("\r\ntx = ");
			AS1_printByteAsChars (tx);
			AS1_printStringEx ("\r\n");
			c = 0;
			break;
		
		//@@test scrittura su eeprom	
		case '1':
			EEPROM_WRITE(&_board_ID,0,1);
			AS1_printStringEx ("eeprom written:");
			AS1_printStringEx ("\r\n");
			break;
		
		//@@test lettura da eeprom
		case '2':
			EEPROM_READ(rd_eeprom,0,1);
			AS1_printStringEx ("eeprom read:");
			AS1_printByteAsChars (rd_eeprom[0]);
			AS1_printStringEx ("\r\n");
			AS1_printWord16AsChars (rd_eeprom[0]);
			AS1_printStringEx ("\r\n");
			break;
	
		//@@test lettura da eeprom
		case '3':
			EEPROM_RDSR(rd_eeprom);
			AS1_printStringEx ("eeprom sr:");
			AS1_printByteAsChars (rd_eeprom[0]);
			AS1_printStringEx ("\r\n");
			break;	

		case 'c':
			axis = (axis < serial_channels-1) ? axis+1 : 0 ;
			AS1_printStringEx ("channel =");
			AS1_printByteAsChars (axis);
			AS1_printStringEx ("\r\n");
			c = 0;
			break;
			
		case 'e':
			AS1_printStringEx ("channel =");
			AS1_printByteAsChars (axis);
			AS1_printStringEx ("\r\n");
				
			if (_control_mode[axis] == MODE_IDLE)
			{
				_control_mode[axis] = MODE_POSITION;
				AS1_printStringEx ("mode = position\r\n");
			}
			else
			{
				_control_mode[axis] = MODE_IDLE;
				AS1_printStringEx ("mode = idle\r\n");
			}
			c = 0;
			break;
			
		case 'x':
			do
				{
					AS1_RecvChar(&d);
				}
			while (d!=0);
			{
				if (d == '1')
				{
					AS1_printStringEx ("channel =");
					AS1_printByteAsChars (axis);
					AS1_printStringEx ("\r\n");
						
					AS1_printStringEx ("position: ");
					AS1_getStringEx (buffer, SMALL_BUFFER_SIZE, true);
					iretval = AS1_atoi (buffer, AS1_strlen(buffer, SMALL_BUFFER_SIZE)); 
					
					AS1_printStringEx ("move: ");
					_set_point[axis] = 0;
					_set_point[axis] = L_deposit_l(iretval);
					AS1_printDWordAsCharsDec (iretval);
					_set_vel[axis] = 10;
					_set_acc[axis] = 0;
					AS1_printStringEx (" 10\r\n");
					init_trajectory (axis, _position[axis], _set_point[axis], _set_vel[axis]);
				}
				else
				if (d == '2')
				{
					AS1_printStringEx ("channel =");
					AS1_printByteAsChars (axis);
					AS1_printStringEx ("\r\n");
						
					abort_trajectory (axis, _position[axis]);
					AS1_printStringEx ("trajectory aborted\r\n");
				}
				else
				if (d == '3')
				{
					AS1_printStringEx ("channel =");
					AS1_printByteAsChars (axis);
					AS1_printStringEx ("\r\n");
			
					if (_pad_enabled[axis] == true)
						PWM_outputPadDisable(axis);
					else
						PWM_outputPadEnable(axis);
						
					_pad_enabled[axis] = !_pad_enabled[axis];
				}

				c = 0;
			}
		
			break;
			
		case 'g':
			AS1_printStringEx ("channel =");
			AS1_printByteAsChars (axis);
			AS1_printStringEx ("\r\n");
		
			ASK_PARM("p gain", &_kp[axis]);
			ASK_PARM("d gain", &_kd[axis]);
			ASK_PARM("scale factor", &_kr[axis]);
			ASK_PARM("offset", &_ko[axis]);
			ASK_PARM("limit", &_pid_limit[axis]);
			
			ASK_PARM("max position", &_max_position[axis]);
			_min_position[axis] = -_max_position[axis];
					
			c = 0;
			break;
			
		case 's':
			AS1_printStringEx ("channel =");
			AS1_printByteAsChars (axis);
			AS1_printStringEx ("\r\n");
			
			AS1_printStringEx ("gain, p= ");
			AS1_printWord16AsChars (_kp[axis]);
			AS1_printStringEx (" d= ");
			AS1_printWord16AsChars (_kd[axis]);
			AS1_printStringEx (" scale= ");
			AS1_printWord16AsChars (_kr[axis]);
			AS1_printStringEx (" offset= ");
			AS1_printWord16AsChars (_ko[axis]);
			AS1_printStringEx (" limit= ");
			AS1_printWord16AsChars (_pid_limit[axis]);
			AS1_printStringEx ("\r\n");

			AS1_printStringEx ("max position= ");
			AS1_printDWordAsChars (_max_position[axis]);
			AS1_printStringEx ("\r\n");
						
			c = 0;
			break;
#endif

		case 'n':
			AS1_printStringEx ("pwm fault 0 = ");
			iretval = getReg (PWMA_PMFSA);
			AS1_printUWord16AsChars (iretval);
			
			AS1_printStringEx (" dismap 0 = ");
			iretval = getReg (PWMA_PMDISMAP1);
			AS1_printUWord16AsChars (iretval);
			
			AS1_printStringEx ("  pwm fault 1 = ");
			iretval = getReg (PWMB_PMFSA);
			AS1_printUWord16AsChars (iretval);

			AS1_printStringEx (" dismap 1 = ");
			iretval = getReg (PWMB_PMDISMAP1);
			AS1_printUWord16AsChars (iretval);
			
			AS1_printStringEx ("\r\n");
			c = 0;
			break;

		case 'v':
			_verbose = !_verbose;
			if (_verbose)
				AS1_printStringEx ("verbose is now ON\r\n");
			else
				AS1_printStringEx ("verbose is now OFF\r\n");
				
			c = 0;
			break;
			
		case 'a':
			AS1_printStringEx ("address [1-15]: ");
			AS1_getStringEx (buffer, SMALL_BUFFER_SIZE, true);
			iretval = AS1_atoi (buffer, AS1_strlen(buffer, SMALL_BUFFER_SIZE)); 
			
			if (iretval >= 1 && iretval <= 15)
			{
				_board_ID = iretval & 0x0f;	
				AS1_printStringEx ("Address changed to: ");
				AS1_printWord16AsChars (iretval);
				AS1_printStringEx ("\r\n");
				set_can_masks();
			}
			
			else
			{
				AS1_printStringEx ("The specified address is not valid!\r\n");				
				AS1_printStringEx ("Current address is: ");
				AS1_printWord16AsChars (_board_ID);
				AS1_printStringEx ("\r\n");				
			}
			
			c = 0;
			break;

		case 'b':
			AS1_printStringEx ("address is ");
			iretval = BYTE_W(_board_ID, 0);
			AS1_printWord16AsChars (iretval);
			AS1_printStringEx ("\r\n");
			
			c = 0;
			break;
		
		case 'A':
			AS1_printStringEx ("additional info (max 32 chars): ");
			AS1_getStringEx   (_additional_info, SMALL_BUFFER_SIZE, true);
			AS1_printStringEx ("additional info set:");
			AS1_printStringEx (_additional_info);
			AS1_printStringEx ("\r\n");
			
			set_additional_info (buffer);
			
			c = 0;
			break;

		case 'B':
			get_additional_info (_additional_info);
			AS1_printStringEx ("additional info is: ");
			AS1_printStringEx (_additional_info);
			AS1_printStringEx ("\r\n");
			
			c = 0;
			break;
			
		case 'w':
			AS1_printStringEx ("writing to FLASH mem\r\n");
			writeToFlash (_flash_addr);
			AS1_printStringEx ("done!\r\n");
			c = 0;
			break;
	
		case 'r':
			AS1_printStringEx ("reading from FLASH mem\r\n");
			readFromFlash (_flash_addr);
			AS1_printStringEx ("done!\r\n");
			c = 0;
			break;
		
	}	/* end switch/case */
#endif
}
