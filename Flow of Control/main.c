#include<stdio.h>

int main()
{
  int counter = 0;
  
  while(counter < 21){
    ++counter;

    if((counter & 1) != 0){
      // do something when the counter is odd
      printf("The number is odd\n");

    }

    else{
      printf("The number is even\n");
    }
  }
  
  
  return 0;
}
