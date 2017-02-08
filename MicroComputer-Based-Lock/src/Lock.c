/*
 * Lock.c
 *
 *  Created on: Feb 8, 2017
 *      Author: Anderson Banihirwe
 *
 *
 *
 */



#include "stm32f4xx.h"
#include "LOCK.h"


void Lock_Init(void){
	volatile unsigned long delay;
	SYSCTL_PRGPIO_R |= 0x01;		// activate clock for port A
	delay = SYSCTL_PRGPIO_R;       // allow time for clock to start
	GPIO_PORTA_DIR_R = 0x80;       // set PA7 to output and PA6 to input
	GPIO_PORTA_DEN_R = 0xFF;       // enable digital port

}



void Lock_Set(int flag){
	if(flag){
		GPIO_PORTA_DATA_R = 0x80;
	}

	else{
		GPIO_PORTA_DATA_R = 0;
	}

}


unsigned long Lock_Input(void){
	return GPIO_PORTA_DATA_R&0x7F;    // 0 to 127
}

