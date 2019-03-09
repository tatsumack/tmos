tmos.bin: tmos.asm
	nasm tmos.asm -o tmos.bin -l tmos.lst

tmos.img: tmos.bin
	mformat -f 1440 -C -B tmos.bin -i tmos.img ::

run: tmos.img
	qemu-system-i386 -drive file=tmos.img,format=raw,index=0,media=disk

clean:
	rm *.bin *.img *.lst
