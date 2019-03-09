; tmos-ipl

CYLS EQU 10             ; read 10 cylinders

    ORG 0x7c00

; floppy disk
    JMP SHORT entry
    DB  0x90
    DB  "TMOS IPL"      ; name of boot sector (8 bytes)
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

; initialize register
entry:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX

; read disk
    MOV AX, 0x0820
    MOV ES, AX
    MOV CH, 0           ; cylinder
    MOV DH, 0           ; head
    MOV CL, 2           ; sector

readloop:
    MOV SI, 0           ; count error

retry:
    MOV AH, 0x02        ; read disk
    MOV AL, 1           ; 1 sector
    MOV BX, 0
    MOV DL, 0x00        ; A drive
    INT 0x13            ; disc bios
    JNC next
    ADD SI, 1           ; increment error count
    CMP SI, 5
    JAE error
    MOV AH, 0x00        ; reset disk
    MOV DL, 0x00
    INT 0x13
    JMP retry

next:
    MOV AX, ES
    ADD AX, 0x0020      ; 0x0020 = 520
    MOV ES, AX
    ADD CL, 1
    CMP CL, 18
    JBE readloop
    MOV CL, 1
    ADD DH, 1
    CMP DH, 2
    JB  readloop
    MOV DH, 0
    ADD CH, 1
    CMP CH, CYLS
    JB  readloop

    MOV [0x0ff0], CH    ; memo read num
    JMP 0xc200          ; exec tmos.sys

error:
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
    DB  "load error"
    DB  0x0a
    DB  0

    TIMES 0x1fe-($-$$) DB 0 ; filled by 0 up to 0x1fe

    DB  0x55, 0xaa
