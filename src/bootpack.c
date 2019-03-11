#include <stdio.h>

#include "bootpack.h"

extern FIFO keyfifo;

void tmos_main(void) {
    init_gdtidt();
    init_pic();

    // enable interrupt
    io_sti();

    unsigned char keybuf[32];
    fifo_init(&keyfifo, 32, (unsigned char *)keybuf);

    io_out8(PIC0_IMR, 0xf9); // enable keyboard interrupt and PIC1
    io_out8(PIC1_IMR, 0xef); // enable mouse interrupt


    init_palette();

    BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;
    init_screen(binfo->vram, binfo->width, binfo->height);

    char mcursor[256];
    init_mouse_cursor8(mcursor, COL8_008484);
    int mx = (binfo->width - 16) / 2;
    int my = (binfo->height - 28 - 16) / 2;
    putblock8_8(binfo->vram, binfo->width, 16, 16, mx, my, mcursor, 16);

    putstring8(binfo->vram, binfo->width, 31, 31, COL8_000000, "TMOS");
    putstring8(binfo->vram, binfo->width, 30, 30, COL8_FFFFFF, "TMOS");


    for (;;) {
        io_cli();
        if (fifo_empty(&keyfifo)) {
            io_stihlt();
            continue;
        }

        int i = fifo_get(&keyfifo);
        io_sti();

        char s[40];
        sprintf(s, "%02X", i);
        draw_rec(binfo->vram, binfo->width, COL8_008484, 0, 16, 15, 31);
        putstring8(binfo->vram, binfo->width, 0, 16, COL8_FFFFFF, s);
    }
}

