#include<stdio.h>

int counter = 0;
int main()
{
  // pointer to an integer in other words p_int is a variable that can hold 
  // addresses of integer variables
  
  int *p_int;
  p_int = &counter;
  
  // Dereferencing, *p_int: the value of the address currently stored in the 
  // p_int pointer 
  while(*p_int < 21){
    ++(*p_int);
  }

  p_int = (int *)0x20002000;
  *p_int = 0xDEADBEEF;
  
  return 0;
}
