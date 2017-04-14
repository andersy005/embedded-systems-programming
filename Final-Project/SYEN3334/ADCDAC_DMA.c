/**
  ******************************************************************************
  * @file    ADC3_DMA.c 
  * @author  Jing Zhang
  * @version V1.0.0
  * @date    14-March-2017
  * @brief   Function body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include <stdio.h>
//#include "aProjHead.h"
    
/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup ADC_ADC3_DMA
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001204C)
#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)
    
#define AdcOffsetA3     ((uint16_t) 2048+59)
#define AdcOffsetA5     ((uint16_t) 2048+44)
#define AdcOffsetB0     ((uint16_t) 2048)
#define AdcOffsetB1     ((uint16_t) 2048)
#define AdcOffsetC2     ((uint16_t) 2048+70)
#define AdcOffsetC3     ((uint16_t) 2048+18)
#define SampleN         ((int) 1000) 

/* Private macro -------------------------------------------------------------*/
struct AdcDatSet{
  int   SecCount;       //Counting of seconds
  float fAdcPA5;
  float fAdcPB0;
  float fAdcPB1;
};

/* Private variables ---------------------------------------------------------*/
/* You can monitor the converted value by adding the variable "ADC3ConvertedValue" 
   to the debugger watch window */
__IO uint16_t ADC3ConvertedValue = 0;
__IO uint32_t ADC3ConvertedVoltage = 0;
__IO uint16_t ADCConvertedValueSet[2];

DAC_InitTypeDef DAC_InitTypeDefinition;

int32_t AccAdcSamples[2];
int     AdcCount = 0, SecNum = 0;
float   AdcQuanti[2];
struct AdcDatSet AdcMeanValueSet;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*
ADC&DMA configuration with channels: PB0, PB1
*/
void ADC_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCConvertedValueSet;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  
  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC regular channel12 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_3Cycles);


 /* Enable DMA request after last transfer (Single-ADC mode) */
//  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
}

/* DAC Initialization */
void DAC_DMA_Config(void)
{
  /* DAC Initialization */
    /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); 
  //DAC_StructInit1(&DAC_InitTypeDefinition);
  /* Initialize the DAC_Trigger member */
  DAC_InitTypeDefinition.DAC_Trigger = DAC_Trigger_Software;
  /* Initialize the DAC_WaveGeneration member */
  DAC_InitTypeDefinition.DAC_WaveGeneration = DAC_WaveGeneration_None;
  /* Initialize the DAC_LFSRUnmask_TriangleAmplitude member */
  DAC_InitTypeDefinition.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
  /* Initialize the DAC_OutputBuffer member */
  DAC_InitTypeDefinition.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
/*DAC is disabled to avoid the conflict of ADC and DAC PA5*/
  DAC_Init(DAC_Channel_2, &DAC_InitTypeDefinition);
  DAC_Cmd(DAC_Channel_2, ENABLE);
}

/* DAC ouput */
void DacConv(int16_t data)
{
   DAC_SetChannel2Data(DAC_Align_12b_L, data);
   DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
}


void AdcDataAcqInit(void)
{ //Initialization of the data acqisitoin
  int i;
  for (i=0; i < 2; i++)
  {
    ADCConvertedValueSet[i]=0;
    AccAdcSamples[i]=0;
    AdcQuanti[i] = 0;
  }
  SecNum = 0;
  AdcCount = SampleN;
}

uint16_t AdcDataAcq(void)
{
  int i;
  //Data acquisition of AD convertere
  AccAdcSamples[0] += ADCConvertedValueSet[0];
  AccAdcSamples[1] += ADCConvertedValueSet[1];


  if (AdcCount-- == 0)
  {
    AdcCount = SampleN;
    for (i=0; i< 2; i++)
    {
      AdcQuanti[i] = (float) AccAdcSamples[i] / (float) SampleN;
      AccAdcSamples[i]=0;
    }
    SecNum++;
    
    AdcMeanValueSet.SecCount = SecNum;
    AdcMeanValueSet.fAdcPB0 = AdcQuanti[0];
    AdcMeanValueSet.fAdcPB1 = AdcQuanti[1];
    
    /*
    AdcMeanValueSet.SecCount = 123;
    AdcMeanValueSet.fAdcPA3 = 0.12;
    AdcMeanValueSet.fAdcPA5 = 3.14;
    AdcMeanValueSet.fAdcPB0 = 10.2;
    AdcMeanValueSet.fAdcPB1 = 1.0;
    AdcMeanValueSet.fAdcPC2 = 2.54;
    AdcMeanValueSet.fAdcPC3 = 0.0123456;
    */
  }
  return ADCConvertedValueSet[0];
}
    
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
