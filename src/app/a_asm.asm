[BITS 32]
    GLOBAL api_win_open
    GLOBAL api_win_putstr
    GLOBAL api_win_drawrec
    GLOBAL api_putstr0
    GLOBAL api_putchar
    GLOBAL api_end

[SECTION .text]

api_win_open:    ; int api_win_open(char* buf, int width, int height, int col_inv, char* title);
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

api_win_putstr:    ; void api_win_putstr(int win, int x, int y, int col, int len, char* str);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBP
    PUSH    EBX
    MOV     EDX, 6
    MOV     EBX, [ESP+20]
    MOV     ESI, [ESP+24]
    MOV     EDI, [ESP+28]
    MOV     EAX, [ESP+32]
    MOV     ECX, [ESP+36]
    MOV     EBP, [ESP+40]
    INT     0x40
    POP     EBX
    POP     EBP
    POP     ESI
    POP     EDI
    RET

api_win_drawrec:    ; void api_win_drawrec(int win, int x0, int y0, int x1, int y1, int col);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBP
    PUSH    EBX
    MOV     EDX, 7
    MOV     EBX, [ESP+20]
    MOV     EAX, [ESP+24]
    MOV     ECX, [ESP+28]
    MOV     ESI, [ESP+32]
    MOV     EDI, [ESP+36]
    MOV     EBP, [ESP+40]
    INT     0x40
    POP     EBX
    POP     EBP
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
