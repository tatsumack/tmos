[BITS 32]
    GLOBAL tmos_main

[SECTION .text]

tmos_main:
    MOV     EDX, 2
    MOV     EBX, msg
    INT     0x40
    MOV     EDX, 4
    INT     0x40

[SECTION .data]
msg:
    DB      "hellooooo, world!!!!!", 0x0a, 0
