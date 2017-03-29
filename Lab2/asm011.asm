;##############################################################################
;
;       Project001/asm011
;       The first project for assembly language program of STM32F407
;       Discovery Kit
;       Jing Zhang, 5/4/2015
;       Test assembly instructions for data move and memory access
;##############################################################################

        NAME    main    ;This line with directive NAME can not begin with the first character of the line
        
        PUBLIC  __iar_program_start
        extern  sintab                          ;lookup table
        public  array
        SECTION var:DATA (6)
        data
        ALIGNRAM        2
var1    DS32    1
var2    DS32     1
var3    DS16    1
array   DS32    64

        section myconst:CONST (2)
        data
table   dc32     12,  34, 56, 78

        SECTION .intvec : CODE (2)
        
count   set 0
memadt  set 0x20000000  ;Ram beginning address

__iar_program_start
        B       main

        
        SECTION .text : CODE (2)

main    NOP
        ;initialize a 32-bit register with data moving instructions
        mov    r0,#4
        mov     r0,#0x2000
        movw    r0,#0x3124
        movt    r0,#0x2000      ;movt will write only the most significant 16-bit of the register without affecting the least significant 16 bit
        movw    r0,#0x4142
        
        movs    r1,r0
        it      mi              ;skip the next instruction if value is 0 or positive. MI is the conditional code for N=1
        mvnmi   r1,r0
        
        mov     r1,r0,ASR#2   ;arithmetic shift right n bits

;PC-related addressing LDR Rn, [PC,#offset].
        ldr     sp,=0x20000300  ;intialize sp register
        ldr     r0,=0x20000010  ;the constant is stored in the program, using pc+offset addressing. Offset unit is 4 bytes

        ;test of immediate offset addressing of memory location
        ldr     r2,=0x12345678
        str     r2,[r0],#4
        ldr     r2,=0x01020304
        str     r2,[r0],#4
        ldr     r2,=0x22334455
        str     r2,[r0],#4
        ldr     r2,=0x66778899
        str     r2,[r0],#4
        ldr     r0,=0x20000010  ;the constant is stored in the program, using pc+offset addressing. Offset unit is 4 bytes
        ldrb    r2,[r0]      ;read a byte from memory into r2. The least significant byte is written while other bytes in r2 are cleared.
        ldrsb   r2,[r0]      ;read a byte from memory into r2. The least significant byte is written while all other bits in r2 are sign extended based on the most significant bit of the written byte.
        ldr     r1,[r0,#0x4]    ;read a word from memory into r1. data in memory stored in little-endian format
        ldr     r2,[r0],#0x4
        ldr     r0,=0x20000010
        str     r1,[r0,#0x4]    ;write a word from r1 into memory loction r0+0x4
        ldrd    r2, r3, [r0]       ;64-bit data read: the word in low address memory written into r2, the word in high address memory written into r3
        strd    r2, r3, [r0,#0x10]      ;64-bit data write: the word in r2 written into low-addressed memory location, the word in r3 written into high-addressed memory location.
        
        ldmdb   r0,{r4-r7}      ;read three 32 bits to r4, r5, r6, r7
        
        ;Stack push and pop
        push    {r2}
        push    {r3, r4}
        pop     {r1, r2}
        pop     {r2}
        
        ;access a constant in program memory (ICode, DCode)
        ldr     r0, =table
        ldr     r2,[r0]
        ;access a variable in system memory (System)
        ldr     r0,=var3
        str     r2,[r0]

        ;access a lookup table for sin function from 0 to 90 degrees
        ldr     r0,=sintab      ;r0 points at the sintab
        mov     r1,#45          ;set the angle at 45 degrees. The value of r1 is the angle in degree. It is used as the index to find the sinsoidal value from the lookup table sintab.
        ldr     r2,[r0, r1,lsl#2]       ;read the value of sin(angle)
        
        ;access a data array in system memory
        ldr     r0,=array
        mov     r1, #64
;        ldr     r2,=0x12345678
        mov     r2,#0           ;clear the whole array
loop1   cbz     r1, loop1exit   ;if the value in r1 is zero, jump to loop1exit. Otherwise, continue the next instruction.
        str     r2, [r0], #4
        subs    r1, #1
        b       loop1           ;unconditional jump to loop1
loop1exit        
        B main

        END

