; asmfunc

[BITS 32]           ; 32bit mode

    GLOBAL io_hlt
    GLOBAL io_cli
    GLOBAL io_sti
    GLOBAL io_stihlt
    GLOBAL io_out8
    GLOBAL io_in8
    GLOBAL io_load_eflags
    GLOBAL io_store_eflags
    GLOBAL load_gdtr
    GLOBAL load_idtr
    GLOBAL asm_inthandler21
    GLOBAL asm_inthandler27
    GLOBAL asm_inthandler2c

    EXTERN inthandler21
    EXTERN inthandler27
    EXTERN inthandler2c

[SECTION .text]

io_hlt: ; void io_hlt(void);
    HLT
    RET

io_cli: ; void io_cli(void);
    CLI
    RET

io_sti:	; void io_sti(void);
    STI
    RET

io_stihlt: ; void io_stihlt(void)
    STI
    HLT
    RET

io_out8: ; void io_out8(int port, int data);
    MOV     EDX, [ESP+4]
    MOV     AL, [ESP+8]
    OUT     DX, AL
    RET

io_in8:	; int io_in8(int port);
    MOV		EDX,[ESP+4]		; port
    MOV		EAX,0
    IN		AL,DX
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

load_gdtr: ; void load_gdtr(int limit, int addr);
    MOV     AX, [ESP+4] ; limit
    MOV     [ESP+6], AX
    LGDT    [ESP+6]
    RET

load_idtr: ; void load_idtr(int limit, int addr);
    MOV     AX, [ESP+4] ; limit
    MOV     [ESP+6], AX
    LIDT    [ESP+6]
    RET

asm_inthandler21:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler21
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    IRETD

asm_inthandler27:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler27
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    IRETD

asm_inthandler2c:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler2c
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    IRETD
