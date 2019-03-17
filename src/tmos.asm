; tmos-boot

BOTPAK	EQU		0x00280000
DSKCAC	EQU		0x00100000
DSKCAC0	EQU		0x00008000


CYLS	EQU		0x0ff0          ; cylinder
LEDS	EQU		0x0ff1          ; keyboard LED
VMODE	EQU		0x0ff2          ; color
SCRNX	EQU		0x0ff4          ; width of screen size
SCRNY	EQU		0x0ff6          ; height of screen size
VRAM	EQU		0x0ff8

		ORG		0xc200

		MOV     BX, 0x4101       ; VBE 640*480
		MOV     AX, 0x4f02
		INT     0x10
		MOV     BYTE [VMODE], 8
		MOV		WORD [SCRNX], 640
		MOV		WORD [SCRNY], 480
		MOV		DWORD [VRAM], 0xfd000000

; keyboard LED
		MOV		AH,0x02
		INT		0x16            ; keyboard BIOS
		MOV		[LEDS],AL

; settings for PIC
		MOV		AL,0xff
		OUT		0x21,AL         ; io_out(PIC0_IMR, 0xff)  disable all interrupts for PIC0
		NOP
		OUT		0xa1,AL         ; io_out(PIC1_IMR, 0xff)  disable all interrupts for PIC1

		CLI                     ; clear interrupts flag

; settings for KBC
		CALL	waitkbdout      ; wait for kbc
		MOV		AL,0xd1
		OUT		0x64,AL         ; io_out(PORT_KEYCMD, KEYCMD_WRITE_OUTPORT)
		CALL	waitkbdout
		MOV		AL,0xdf
		OUT		0x60,AL         ; io_out(PORT_KEYDAT, KBC_OUTPORT_A20G_ENABLE) enable A20GATE to extend memory spaces over 1MB
		CALL	waitkbdout

; settings for GDT
		LGDT	[GDTR0]
		MOV		EAX,CR0
		AND		EAX,0x7fffffff  ; disable paging
		OR		EAX,0x00000001  ; transit to protect mode
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8          ; RW segment
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; move bootpack
		MOV		ESI,bootpack
		MOV		EDI,BOTPAK
		MOV		ECX,512*1024/4
		CALL	memcpy

; move boot sector
		MOV		ESI,0x7c00
		MOV		EDI,DSKCAC
		MOV		ECX,512/4
		CALL	memcpy

; move rest
		MOV		ESI,DSKCAC0+512
		MOV		EDI,DSKCAC+512
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4
		SUB		ECX,512/4
		CALL	memcpy

; exec bootpack
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3
		SHR		ECX,2
		JZ		skip
		MOV		ESI,[EBX+20]
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy
		RET


		ALIGN	16, DB 0
GDT0:
		TIMES   8 DB 0
		DW		0xffff,0x0000,0x9200,0x00cf
		DW		0xffff,0x0000,0x9a28,0x0047

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: