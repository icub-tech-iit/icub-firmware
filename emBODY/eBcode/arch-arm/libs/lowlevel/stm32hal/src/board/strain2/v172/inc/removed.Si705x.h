/****************************************************
*  MTB4 board - iCub Facility - 2017                *
*  Si705x configuration                             *
*                                                   *
*  Written by Andrea Mura                           *
*  <andrea.mura@iit.it>                             *
****************************************************/

#error DONT USE FOR stm32hal.lib

#include "stm32l4xx_hal.h"

#ifndef Si705x_H
#define Si705x_H

extern const uint8_t Si705x_I2C_ADDRESS;
extern uint8_t Si705x_I2C1_RxBuffer[2];
extern uint8_t Si705x_I2C2_RxBuffer[2];

void Si705x_init(uint8_t TS);
void Si705x_deinit(void);
void Si705x_ReadTemperature(uint8_t TS);

#endif
