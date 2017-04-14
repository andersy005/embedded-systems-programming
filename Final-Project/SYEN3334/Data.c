//########################################################################################
//
// FILE:	Data.c for STM32F4xx
//
// TITLE:	Data sample and system control
//
//########################################################################################
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|============================================================
//  v1.0| Dec.12,2016 | J.Z. | version developed from a_control.c
//########################################################################################

#include "stm32f4_discovery.h"
#include "aProjHead.h"
#include "SciCom.h"

void DataSample(void);
char NewSample = 0; 
int ADCCnt = 0, ADCbuf[1024];

void DataSample(void)
{
	int i, j, m, *p;
	if (NewSample == 1)
	{
		//Data saved in little-endian format.
		m = 0;
		for (i=0; i<DataSetCom.DataNum; i++)
		{
			p = (int*) DataSetCom.Addr[i];
			for (j=0; j<DataSetCom.WrdNum[i]; j++)
			{
				ADCbuf[m++] = (char) *p;
				ADCbuf[m++] = (char) *p>>8;
				p++;
			}
		}
		DataSetCom.ByteNum = m;	//Set the number of sampled bytes.
		NewSample = 2;
	}
	else if (NewSample == 5)
	{
		if (CurveCom.tcnt++ >= CurveCom.delay)
		{
			CurveCom.tcnt = 0;
			for (i=0; i<CurveCom.DataNum; i++)
			{
				p = (int*) CurveCom.Addr[i];
				for (j=0; j<CurveCom.WrdNum[i]; j++)
				{
					ADCbuf[ADCCnt++] = *p;
					p++;
				}
			}

			if (ADCCnt >= 1000) NewSample = 6;
		}
	}
}


//===========================================================================
// End of SourceCode.
//===========================================================================

