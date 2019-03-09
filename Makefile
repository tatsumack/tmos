ipl.bin: ipl.asm
	nasm ipl.asm -o ipl.bin -l ipl.lst

ipl.img: ipl.bin
	mformat -f 1440 -C -B ipl.bin -i ipl.img ::

run: ipl.img
	qemu-system-i386 -drive file=ipl.img,format=raw,index=0,if=floppy

clean:
	rm *.bin *.img *.lst
