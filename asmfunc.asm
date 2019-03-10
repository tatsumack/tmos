; asmfunc

[BITS 32]           ; 32bit mode

    GLOBAL io_hlt
    GLOBAL io_cli
    GLOBAL io_out8
    GLOBAL io_load_eflags
    GLOBAL io_store_eflags

[SECTION .text]

io_hlt: ; void io_hlt(void);
    HLT
    RET

io_cli: ; void io_cli(void);
    CLI
    RET

io_out8: ; void io_out8(int port, int data);
    MOV EDX, [ESP+4]
    MOV AL, [ESP+8]
    OUT DX, AL
    RET

io_load_eflags: ; int io_load_eflags(void);
    PUSHFD      ; push eflags to stack
    POP     EAX
    RET

io_store_eflags: ; void io_store_eflags(int eflags);
    MOV     EAX, [ESP+4]
    PUSH    EAX
    POPFD                   ; pop eflags from stack
    RET
