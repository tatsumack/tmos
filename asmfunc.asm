; asmfunc

[BITS 32]           ; 32bit mode

    GLOBAL io_hlt

[SECTION .text]

io_hlt:
    HLT
    RET