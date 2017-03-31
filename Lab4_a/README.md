Lab 04a: Programming for GPIO Port of STM32F4xx 
========================================

# Objective
The objective of this lab is to learn the configuration of general-purpose inputs and outputs of the microcontroller STM32F4xx and create a new project with existing source files provided by ARM and STM. 

# Tasks:

-	Create a new project Lab4 to replace the example project IO_Toggle
-	Modify the configuration of GPIOD in the function main().
-	Test GPIO inputs and outputs to confirm the functions

# Procedure:

1.	Follow the description of “Guide to Create a New C Project Using Existing Source File” to create a new project Lab4 to replace the example project IO_Toggle in the folder ..\Projects\Lab4.
2.	Test the project to make sure that the function of the project are the same as the project IO_Toggle.
3.	Modify the main function in source code file main.c in the project folder to add the following new functions:

      - Write code to add configuration for 

            a.	GPIOD_Pin1:2: GPIO input with pull-up
            b.	GPIOD_Pin5:6: GPIO input with pull-down
            c.	GPIOD_Pin8:9: GPIO output with OD, fast speed, no pull down/up

      -	Write code to test all GPIO inputs and outputs to confirm the functions
4.	Test added GPIO functions

