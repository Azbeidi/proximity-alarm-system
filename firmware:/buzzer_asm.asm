
        ; To include names declared in C
        .cdecls "buzzer.h"

        .text

; Declare a field to store GPIO_PORTC_BASE (32-bit value) in program memory
_GPIO_PORTC_BASE     .field  GPIO_PORTC_BASE


        .global BuzzerOn
        .asmfunc
BuzzerOn
        PUSH    {LR}            ; save the return address

        ; GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5)
        ;
        LDR     r0, _GPIO_PORTC_BASE
        MOV     r1, #GPIO_PIN_5
        MOV     r2, #GPIO_PIN_5
        BL      GPIOPinWrite

        POP     {PC}            ; return
        .endasmfunc


        .global BuzzerOff
        .asmfunc
BuzzerOff
        PUSH    {LR}            ; save the return address

        ; Write 0 to the GPIO pin that the buzzer uses
        ; GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0)
        LDR     r0, _GPIO_PORTC_BASE
        MOV     r1, #GPIO_PIN_5
        MOV     r2, #0
        BL      GPIOPinWrite

        POP     {PC}            ; return
        .endasmfunc

