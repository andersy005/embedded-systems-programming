
/* Includes */
#include "stm32f4xx.h"

GPIO_InitTypeDef GPIO_LED;

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  int i = 0;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_LED.GPIO_Pin = GPIO_Pin_12;
  GPIO_LED.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_LED.GPIO_OType = GPIO_OType_PP;
  GPIO_LED.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOD, &GPIO_LED);

  GPIO_WriteBit(GPIOD, GPIO_Pin_12, Bit_SET);



  while (1)
  {
	i++;
  }
}
