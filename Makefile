CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
BIN_PATH = bin
SRC_PATH = src
LIB_PATH = golibc/
INC_PATH = golibc/

SRCS = $(wildcard ${SRC_PATH}/*.c)
OBJS = $(addprefix ${BIN_PATH}/, $(notdir $(SRCS:.c=.o)))

${BIN_PATH}/asmfunc.o: ${SRC_PATH}/asmfunc.asm
	nasm -felf32 -o $@ $<

${BIN_PATH}/%.bin: ${SRC_PATH}/%.asm
	nasm $< -o $@

${BIN_PATH}/%.o: ${SRC_PATH}/%.c
	$(CC) -c -m32 -fno-pic -I${INC_PATH} -o $@ $<

${BIN_PATH}/bootpack.bin: ${OBJS} ${BIN_PATH}/asmfunc.o
	$(LD) -m elf_i386 -e tmos_main -o ${BIN_PATH}/bootpack.bin -T${SRC_PATH}/tmos.ls ${BIN_PATH}/asmfunc.o ${OBJS} -static -L$(LIB_PATH) -lgolibc

${BIN_PATH}/tmos.sys: ${BIN_PATH}/tmos.bin ${BIN_PATH}/bootpack.bin
	cat ${BIN_PATH}/tmos.bin ${BIN_PATH}/bootpack.bin > ${BIN_PATH}/tmos.sys

${BIN_PATH}/tmos.img: ${BIN_PATH}/ipl.bin ${BIN_PATH}/tmos.sys ${BIN_PATH}/hello.bin
	mformat -f 1440 -C -B ${BIN_PATH}/ipl.bin -i ${BIN_PATH}/tmos.img ::
	mcopy -i ${BIN_PATH}/tmos.img ${BIN_PATH}/tmos.sys ::
	mcopy -i ${BIN_PATH}/tmos.img ${SRC_PATH}/ipl.asm ::
	mcopy -i ${BIN_PATH}/tmos.img Makefile ::
	mcopy -i ${BIN_PATH}/tmos.img ${BIN_PATH}/hello.bin ::

run: ${BIN_PATH}/tmos.img
	qemu-system-i386 -m 32 -drive file=${BIN_PATH}/tmos.img,format=raw,index=0,if=floppy

clean:
	rm bin/*
