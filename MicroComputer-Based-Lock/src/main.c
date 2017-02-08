

/* Includes */
#include "stm32f4xx.h"
#include "LOCK.h"

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */


/* List of the inputs and outputs. Specify the range of values and their */
/* significance */

#define GPIO_PORTA_DATA_R        (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R         (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_DEN_R         (*((volatile unsigned long *)0x4000451C))
#define SYSCTL_PRGPIO_R          (*((volatile unsigned long *)0x400FEA08))

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


  /* Infinite loop */
  while (1)
  {
	i++;
  }
}
