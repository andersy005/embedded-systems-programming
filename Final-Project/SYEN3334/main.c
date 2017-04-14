/**
  ******************************************************************************
  * @file    TIM_PWM_Output/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
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
/* ZJ: This firmware is developed for the class SYEN3334. 4/11/2017
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include "SciCom.h"
#include "Data.h"
#include "ADCDAC_DMA.h"
//#include "aProjHead.h"

/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup TIM_PWM_Output
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint16_t dacdata;
int iSysCtrlWrd;
float fTestData1, fTestData2;

static __IO uint32_t SysTickCnt = 0, SysTickCntOld = 0;


/* Private function prototypes -----------------------------------------------*/
void GPIO_Config(void); //modified version by ZJ, 11/26/16
void IRQ_Config(void); //Configuration of global interrupts

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t SysTickLoadVal;
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */


  /*Configure SysTick*/
  SysTickLoadVal = SystemCoreClock / 1000; //SystemCoreclock = 168000000
  
  if (SysTick_Config(SysTickLoadVal))  
  { 
    /* Capture error */ 
    iSysCtrlWrd = 0;
    while (1);
  }
  
  GPIO_Config();
  USART6_Configure(); //Initialization of serial port USART6
  IRQ_Config();
 
  //ADC and DAC Config;
  ADC_DMA_Config();
  DAC_DMA_Config();
  AdcDataAcqInit();
  dacdata = 0;
  fTestData1 = 3.14156;
  fTestData2 = -186.23;
  while (1)
  {
    
    if (SysTickCntOld != SysTickCnt)
    {
      SysTickCntOld = SysTickCnt; //Update SysTickCnt
      DataSample();     //Process data sample
      dacdata = AdcDataAcq()<<4;        //Read ADC 
      ADC_SoftwareStartConv(ADC1);      //Software start AD conversion
      DacConv(dacdata);         //Set DA conversion
    }
    CmdDecode();  //SCI communication 
  }
}


/**
  * @brief  Configure the TIM3 Ouput Channels.
  * @param  None
  * @retval None
  */
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA, GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
 
   /* GPIOA Configuration:  DAC_Channel2 (PA5) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    /* Configure ADC Channel 8&9 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    /* GPIOC Configuration:  USART6_TX (PC6), USART6_RX (PC7) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
  
      /* Connect USART6 pins to AF8 */  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);
}

/* Interrupt Configuration */
void IRQ_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Enable the USART6 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
 /* SysTick Interrupt Service Routine */
void SysTickISR(void)
{
  SysTickCnt++;        //Increment interrupts
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
