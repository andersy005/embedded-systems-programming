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
extern __IO uint16_t ADC3ConvertedValue;
extern __IO uint32_t ADC3ConvertedVoltage;
extern __IO uint16_t ADCConvertedValueSet[6];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void ADC3_CH12_DMA_Config(void);
extern void ADC_DMA_Config(void);
extern void AdcDataAcq(void);
extern void AdcDataAcqInit(void);
extern void DAC_DMA_Config(void);
extern void DacConv(int16_t data);