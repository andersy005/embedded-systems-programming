# Objective

The objective of this lab is to use IAR Embedded Workbench IDE to create a new project for assembly program for the microcontroller evaluation board STM32F4 Discovery. 

Tasks:

•	Create a new project for assembly program

•	Manage project by adding files into the project

•	Debug the project

•	Access a lookup table

# Debug the Project

| Name   | Memory addr | Bus    |
|--------|-------------|--------|
| var3   | 0x20000008  | system |
| array  | 0x2000000A  | system |
| table  | 0x08000238  | Dcode  |
| sintab | 0x08000004  | Icode  |

# Test The lookup table

The lookup table in sintable.asm is a fixed-point lookup table for sinusoidal function. The lookup table includes ninety 32-bit integers which are corresponding to the angle from 0 to 90°. They are calculated as:

**value=round[2^31*sin⁡(angle)]**


By debugging the program we find the values for angle equal to 30 degrees, 45 degrees, and 60 degrees and fill the form below.
