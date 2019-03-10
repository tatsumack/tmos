CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld

ipl.bin: ipl.asm
	nasm ipl.asm -o ipl.bin

tmos.bin: tmos.asm
	nasm tmos.asm -o tmos.bin

asmfunc.o: asmfunc.asm
	nasm -felf32 -o asmfunc.o asmfunc.asm

bootpack.o: bootpack.c
	$(CC) -c -m32 -fno-pic -o bootpack.o bootpack.c

bootpack.bin: bootpack.o asmfunc.o
	$(LD) -m elf_i386 -e tmos_main -o bootpack.bin -Ttmos.ls bootpack.o asmfunc.o

tmos.sys: tmos.bin bootpack.bin
	cat tmos.bin bootpack.bin > tmos.sys

tmos.img: ipl.bin tmos.sys
	mformat -f 1440 -C -B ipl.bin -i tmos.img ::
	mcopy -i tmos.img tmos.sys ::

run: tmos.img
	qemu-system-i386 -drive file=tmos.img,format=raw,index=0,if=floppy

clean:
	rm *.bin *.img *.sys
