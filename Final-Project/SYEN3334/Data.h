//########################################################################################
//
// FILE:	Data.h for STM32F4xx
//
// TITLE:	Data sample and system control
//
//########################################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|============================================================
//  v1.0| Dec.12,2016 | J.Z. | version developed from a_control.c
//########################################################################################

extern void DataSample(void);
extern char NewSample; 
extern int ADCCnt, ADCbuf[1024];