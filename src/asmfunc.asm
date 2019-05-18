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
    GLOBAL load_cr0
    GLOBAL store_cr0
    GLOBAL load_tr
    GLOBAL load_gdtr
    GLOBAL load_idtr
    GLOBAL far_jmp
    GLOBAL far_call
    GLOBAL start_app
    GLOBAL asm_inthandler0c
    GLOBAL asm_inthandler0d
    GLOBAL asm_inthandler20
    GLOBAL asm_inthandler21
    GLOBAL asm_inthandler27
    GLOBAL asm_inthandler2c
    GLOBAL asm_memtest
    GLOBAL asm_tmos_api
    GLOBAL asm_end_app

    EXTERN inthandler0c
    EXTERN inthandler0d
    EXTERN inthandler20
    EXTERN inthandler21
    EXTERN inthandler27
    EXTERN inthandler2c
    EXTERN tmos_api

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

load_cr0: ; int load_cr0(void);
    MOV     EAX, CR0
    RET

store_cr0: ; void store_cr0(int cr0);
    MOV     EAX, [ESP+4]
    MOV     CR0, EAX
    RET

load_tr: ; void load_tr(int tr);
    LTR     [ESP+4]     ;tr
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

far_jmp: ; void far_jmp(int eip, int cs);
    JMP     FAR [ESP+4]
    RET

far_call: ; void far_call(int eip, int cs);
    CALL    FAR [ESP+4]
    RET

asm_inthandler0c:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler0c
    CMP     EAX, 0
    JNE     end_app
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    ADD     ESP, 4
    IRETD

asm_inthandler0d:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler0d
    CMP     EAX, 0
    JNE     end_app
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    ADD     ESP, 4
    IRETD


asm_inthandler20:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		EAX,ESP
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	inthandler20
    POP		EAX
    POPAD
    POP		DS
    POP		ES
    IRETD

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

asm_memtest: ; int asm_memtest(int start, int end);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     ESI, 0xaa55aa55
    MOV     EDI, 0x55aa55aa
    MOV     EAX, [ESP + 12 + 4]     ; i = start
asm_memtest_loop:
    MOV     EBX, EAX
    ADD     EBX, 0xffc              ; check last 4 bytes
    MOV     EDX, [EBX]
    MOV     [EBX], ESI
    XOR     DWORD [EBX], 0xffffffff
    CMP     EDI, [EBX]
    JNE     asm_memtest_break
    XOR     DWORD [EBX], 0xffffffff
    CMP     ESI, [EBX]
    JNE     asm_memtest_break
    ADD     EAX, 0x1000
    CMP     EAX, [ESP + 12 + 8]
    JBE     asm_memtest_loop
    JMP     asm_memtest_fin
asm_memtest_break:
    MOV     [EBX], EDX              ; store original value
asm_memtest_fin:
    POP     EBX
    POP     ESI
    POP     EDI
    RET

asm_tmos_api:
    STI
    PUSH    DS
    PUSH    ES
    PUSHAD              ; save register
    PUSHAD              ; arguments for tmos_api
    MOV     AX, SS
    MOV     DS, AX
    MOV     ES, AX
    CALL    tmos_api
    CMP     EAX, 0
    JNE     end_app
    ADD     ESP, 32
    POPAD
    POP     ES
    POP     DS
    IRETD
end_app:
    MOV     ESP, [EAX]  ; eax is tss.esp0
    POPAD
    RET                 ; return to cmd_app

start_app: ; void start_app(int eip, int cs, int esp, int ds, int* tss_esp0)
    PUSHAD
    MOV     EAX, [ESP+36]       ; eip
    MOV     ECX, [ESP+40]       ; cs
    MOV     EDX, [ESP+44]       ; esp
    MOV     EBX, [ESP+48]       ; ds/ss
    MOV     EBP, [ESP+52]       ; tss.esp0
    MOV     [EBP], ESP          ; save esp of os
    MOV     [EBP+4], SS         ; save ss of os
    MOV     ES, BX
    MOV     DS, BX
    MOV     FS, BX
    MOV     GS, BX
    OR      ECX, 3
    OR      EBX, 3
    PUSH    EBX
    PUSH    EDX
    PUSH    ECX
    PUSH    EAX
    RETF                        ; NOTE os can't call app.

asm_end_app:
    MOV     ESP, [EAX]          ; eax is tss.esp0
    MOV     DWORD [EAX+4], 0
    POPAD
    RET                 ; return to cmd_app

