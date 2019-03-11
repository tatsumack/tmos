#include <stdio.h>

#include "bootpack.h"

extern FIFO keyfifo;

void enable_mouse(void);

void init_keyboard(void);

void tmos_main(void) {
    init_gdtidt();
    init_pic();

    // enable interrupt
    io_sti();

    unsigned char keybuf[32];
    fifo_init(&keyfifo, 32, (unsigned char*) keybuf);

    io_out8(PIC0_IMR, 0xf9); // enable keyboard interrupt and PIC1
    io_out8(PIC1_IMR, 0xef); // enable mouse interrupt

    init_keyboard();

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

    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo_empty(&keyfifo)) {
            io_stihlt();
        } else {
            int i = fifo_get(&keyfifo);
            io_sti();

            char s[40];
            sprintf(s, "%02X", i);
            draw_rec(binfo->vram, binfo->width, COL8_008484, 0, 16, 15, 31);
            putstring8(binfo->vram, binfo->width, 0, 16, COL8_FFFFFF, s);
        }
    }
}

#define PORT_KEYDAT                0x0060
#define PORT_KEYSTA                0x0064
#define PORT_KEYCMD                0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE        0x60
#define KBC_MODE                0x47

void wait_kbc_sendready(void) {
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

void init_keyboard(void) {
    wait_kbc_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_kbc_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

#define KEYCMD_SENDTO_MOUSE        0xd4
#define MOUSECMD_ENABLE            0xf4

void enable_mouse(void) {
    wait_kbc_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_kbc_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}
