[BITS 32]
    GLOBAL api_openwin
    GLOBAL api_putstr0
    GLOBAL api_putchar
    GLOBAL api_end

[SECTION .text]

api_openwin:    ; int api_openwin(char* buf, int width, int height, int col_inv, char* title);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     EDX, 5
    MOV     EBX, [ESP+16]
    MOV     ESI, [ESP+20]
    MOV     EDI, [ESP+24]
    MOV     EAX, [ESP+28]
    MOV     ECX, [ESP+32]
    INT     0x40
    POP     EBX
    POP     ESI
    POP     EDI
    RET

api_putstr0:    ; voi api_putstr0(char* s);
    PUSH    EBX
    MOV     EDX, 2
    MOV     EBX, [ESP+8]
    INT     0x40
    POP     EBX
    RET

api_putchar:    ; void api_putchar(int c);
    MOV     EDX, 1
    MOV     AL, [ESP+4]     ;c
    INT     0x40
    RET

api_end:        ; void api_end();
    MOV     EDX, 4
    INT     0x40
