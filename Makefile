CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
BIN_DIR = bin
LIBPATH = golibc/
INCPATH = golibc/

${BIN_DIR}/ipl.bin: ipl.asm
	nasm ipl.asm -o ${BIN_DIR}/ipl.bin

${BIN_DIR}/tmos.bin: tmos.asm
	nasm tmos.asm -o ${BIN_DIR}/tmos.bin

${BIN_DIR}/asmfunc.o: asmfunc.asm
	nasm -felf32 -o ${BIN_DIR}/asmfunc.o asmfunc.asm

${BIN_DIR}/bootpack.o: bootpack.c
	$(CC) -c -m32 -fno-pic -I$(INCPATH) -o ${BIN_DIR}/bootpack.o bootpack.c

${BIN_DIR}/ascii_fonts.o: ascii_fonts.c
	$(CC) -c -m32 -fno-pic -I$(INCPATH) -o ${BIN_DIR}/ascii_fonts.o ascii_fonts.c

${BIN_DIR}/bootpack.bin: ${BIN_DIR}/bootpack.o ${BIN_DIR}/asmfunc.o ${BIN_DIR}/ascii_fonts.o
	$(LD) -m elf_i386 -e tmos_main -o ${BIN_DIR}/bootpack.bin -Ttmos.ls ${BIN_DIR}/bootpack.o ${BIN_DIR}/asmfunc.o ${BIN_DIR}/ascii_fonts.o -static -L$(LIBPATH) -lgolibc

${BIN_DIR}/tmos.sys: ${BIN_DIR}/tmos.bin ${BIN_DIR}/bootpack.bin
	cat ${BIN_DIR}/tmos.bin ${BIN_DIR}/bootpack.bin > ${BIN_DIR}/tmos.sys

${BIN_DIR}/tmos.img: ${BIN_DIR}/ipl.bin ${BIN_DIR}/tmos.sys
	mformat -f 1440 -C -B ${BIN_DIR}/ipl.bin -i ${BIN_DIR}/tmos.img ::
	mcopy -i ${BIN_DIR}/tmos.img ${BIN_DIR}/tmos.sys ::

run: ${BIN_DIR}/tmos.img
	qemu-system-i386 -drive file=${BIN_DIR}/tmos.img,format=raw,index=0,if=floppy

clean:
	rm bin/*
