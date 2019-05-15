[BITS 32]           ; 32bit mode
    MOV     ECX, msg
putloop:
    MOV     AL, [CS:ECX]
    CMP     AL, 0
    JE      fin
    INT     0x40
    ADD     ECX, 1
    JMP     putloop
fin:
    RETF
msg:
    DB      "hello", 0
