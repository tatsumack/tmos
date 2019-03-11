#include <stdio.h>

#include "bootpack.h"

void tmos_main(void) {
    init_gdtidt();
    init_pic();

    // enable interrupt
    io_sti();

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

    char s[40];
    sprintf(s, "width = %d", binfo->width);
    putstring8(binfo->vram, binfo->width, 16, 64, COL8_FFFFFF, s);

    io_out8(PIC0_IMR, 0xf9); // enable keyboard interrupt and PIC1
    io_out8(PIC1_IMR, 0xef); // enable mouse interrupt

    for (;;) {
        io_hlt();
    }
}

