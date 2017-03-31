;##############################################################################
;       Project01/asm010
;       The first project for assembly language program of STM32F407
;       Discovery Kit
;       Jing Zhang, 1/17/2015
;       An example of ARM assembly program to access GPIOs
;       This is an assembly version for IO_Toggle C program example
;##############################################################################
        NAME    main
        PUBLIC  __iar_program_start
          
        SECTION .intvec : CODE (2)
;__vector_table:
    ;    SECTION .text : CODE (2)
__iar_program_start
        B       main
        
    ;    CODE32
;Define constants        
count   set 0x000729AD //0x0100000   ;for a delay of about 0.8 second
memadt  set 0x20000000  ;Ram beginning address
;Defintiion of RCC registers
RCC     set 0x40023800
RCC_AHB1ENR set RCC + 0x30
;Definition of the registers for GPIOD
GPIOD   set 0x40020C00
MODERD  set GPIOD + 0
OTYPERD set GPIOD + 0x04
OSPEEDRD        set GPIOD + 0x08
PUPDRD  set GPIOD + 0x0C
IDRD    set GPIOD + 0x10
ODRD    set GPIOD + 0x14
BSSRD   set GPIOD + 0x18
LCKRD   set GPIOD + 0x1C
AFRLD   set GPIOD + 0x20
AFRHD   set GPIOD + 0x24
        
        SECTION .text : CODE (2)
main    NOP
        ;load a 32-bit immediate number to reigster r0. This can replace above 
        ;two instruction to initialize the 32-bit register.
        ldr     sp,=0x20000300  ;intialize sp register
        ;enable GPIOD peripheral clock
        ldr     r0,=RCC_AHB1ENR
        ldr     r1,[r0]
        orr     r1,r1,#00000008
        str     r1,[r0]
        ;initialize GPIOD
        ;set the pins as output
        ldr     r0,=MODERD;
        ldr     r1,[r0]
        and     r1,r1,#0x00FFFFFF
        orr      r1,r1,#0x55000000
        str     r1,[r0]
        ;Set the pins as output without drain output
        ldr     r0,=OTYPERD
        ldr     r1,[r0]
        and     r1,r1,#0xFFFF0FFF
        str     r1,[r0]
        ;set the output pins in high-speed mode
        ldr     r0,=OSPEEDRD
        ldr     r1,[r0]
        and     r1,r1,#0x00FFFFFF
        orr      r1,r1,#0xFF000000
        str     r1,[r0]
        ;set no pull at the output pins
        ldr     r0,=PUPDRD
        ldr     r1,[r0]
        and     r1,r1,#0x00FFFFFF
        str     r1,[r0]
        ldr     r0,=ODRD;
loop0:
        ;set GPIOD_Pin12
        ldr     r2,=0x00001000
        bl      ioupdate
        bl      delay
        ;set GPIOD_Pin13
        ldr     r2,=0x00003000
        bl      ioupdate
        bl      delay
        ;set GPIOD_Pin14
        ldr     r2,=0x00007000
        bl      ioupdate
        bl      delay
        ;set GPIOD_Pin15
        ldr     r2,=0x0000F000
        bl      ioupdate
        bl      delay
        ;clear GPIOD_Pin12:15
        ldr     r2,=0x00000000
        bl      ioupdate
        bl      delay
        b       loop0
        
ioupdate: ;function to update GPIO output
        push    {LR}            ;push the link register with return address
        push    {r1}
        ldr     r1,[r0]
        and     r1,r1,#0xFFFF0FFF
        orr     r1,r1,r2
        str     r1,[r0]
        pop     {r1}
        pop     {PC}

delay:   ;delay function
        push    {LR}            ;push the link register with return address
        push    {r3}
        ldr     r3,=count
delaylp1:
        cbz     r3, return1
        nop
        nop
        subs    r3,#1
        b       delaylp1
return1:
        pop     {r3}
        pop     {PC}            ;pop the return address
    
        END