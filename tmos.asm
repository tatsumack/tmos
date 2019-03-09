; tmos
; TAB=4

    ORG 0x7c00

    ; floppy disk
    JMP entry
    DB  0x90

    DB  "HELLOIPL"      ; name of boot sector (8 bytes)
    DW  512             ; size of a sector
    DB  1               ; size of cluster
    DW  1               ; where FAT starts
    DB  2               ; number of FAT
    DW  224             ; size of a root directory
    DW  2880            ; size of this drive
    DB  0xf0            ; type of media
    DW  9               ; length of FAT
    DW  18              ; number of sectors in a track
    DW  2               ; number of head
    DD  0               ; number of partition
    DD  2880            ; size of this drive
    DB  0, 0, 0x29      ; magic number
    DD  0xffffffff      ; volume serial number
    DB  "TMOS       "   ; name of disk (11 bytes)
    DB  "FAT12   "      ; name of format (8 bytes)
    TIMES 18 DB 0       ; reserve 18 bytes

entry:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX
    MOV ES, AX

    MOV SI, msg

putloop:
    MOV AL, [SI]
    ADD SI, 1
    CMP AL, 0
    JE  fin
    MOV AH, 0x0e        ; display a character
    MOV BX, 15          ; color code
    INT 0x10            ; video BIOS
    JMP putloop

fin:
    HLT
    JMP fin

msg:
    DB  0x0a, 0x0a      ; 2 new line codes
    DB  "Hello, World!"
    DB  0x0a
    DB  0

    TIMES 0x1fe-($-$$) DB 0 ;

    DB		0x55, 0xaa

    DB	    0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
    TIMES   4600 DB 0
    DB	    0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
    TIMES	1469432 DB 0