ipl.bin: ipl.asm
	nasm ipl.asm -o ipl.bin

tmos.sys: tmos.asm
	nasm tmos.asm -o tmos.sys

tmos.img: ipl.bin tmos.sys
	mformat -f 1440 -C -B ipl.bin -i tmos.img ::
	mcopy -i tmos.img tmos.sys ::

run: tmos.img
	qemu-system-i386 -drive file=tmos.img,format=raw,index=0,if=floppy

clean:
	rm *.bin *.img *.lst
