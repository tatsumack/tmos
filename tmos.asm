; tmos

; boot info
CYLS    EQU 0x0ff0
LEDS    EQU 0x0ff1              ; keyboard LED
VMODE   EQU 0x0ff2              ; color
SCRNX   EQU 0x0ff4              ; width of screen size
SCRNY   EQU 0x0ff6              ; height of screen size
VRAM    EQU 0x0ff8

        ORG 0xc200

        MOV AL, 0x13            ; vga graphics, 320 * 200, 8bit color
        MOV AH, 0x00            ; change display mode
        INT 0x10

        MOV BYTE [VMODE], 8
        MOV WORD [SCRNX], 320
        MOV WORD [SCRNY], 200
        MOV DWORD [VRAM], 0x000a0000

;keyboard LED
        MOV AH, 0x02
        INT 0x16                ; keyboard bios
        MOV [LEDS], AL

fin:
        HLT
        JMP fin