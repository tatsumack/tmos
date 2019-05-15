[BITS 32]           ; 32bit mode
    MOV     AL, 'T'
    INT     0x40
    MOV     AL, 'M'
    INT     0x40
    MOV     AL, 'O'
    INT     0x40
    MOV     AL, 'S'
    INT     0x40
    RETF
