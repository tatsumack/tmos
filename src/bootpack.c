#include <stdio.h>

#include "bootpack.h"

extern FIFO keyfifo;
extern FIFO mousefifo;

void tmos_main(void) {
    init_gdtidt();
    init_pic();

    // enable interrupt
    io_sti();

    uchar keybuf[32];
    fifo_init(&keyfifo, 32, (uchar*) keybuf);

    uchar mousebuf[128];
    fifo_init(&mousefifo, 128, (uchar*) mousebuf);

    io_out8(PIC0_IMR, 0xf9); // enable keyboard interrupt and PIC1
    io_out8(PIC1_IMR, 0xef); // enable mouse interrupt

    init_keyboard();

    init_palette();

    BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;
    init_screen(binfo->vram, binfo->width, binfo->height);

    // mouse
    char mcursor[256];
    init_mouse_cursor8(mcursor, COL8_008484);
    int mx = (binfo->width - 16) / 2;
    int my = (binfo->height - 28 - 16) / 2;
    putblock8_8(binfo->vram, binfo->width, 16, 16, mx, my, mcursor, 16);
    MouseDec mdec;
    enable_mouse(&mdec);

    // memory
    {
        uint memtotal = memtest(0x00400000, 0xbfffffff);

        MemoryManager* memman = (MemoryManager*) ADR_MEMMAN;
        memman_init(memman);
        memman_free(memman, 0x00001000, 0x0009e000);
        memman_free(memman, 0x00400000, memtotal - 0x00400000);

        char membuf[40];
        sprintf(membuf, "memory_total: %dMB  free_total: %dKB", memtotal / (1024 * 1024), memman_total_free_size(memman) / 1024);
        putstring8(binfo->vram, binfo->width, 4, 32, COL8_FFFFFF, membuf);
    }

    for (;;) {
        io_cli();
        if (!fifo_empty(&keyfifo)) {
            int i = fifo_get(&keyfifo);
            io_sti();

            char s[40];
            sprintf(s, "%02X", i);
            draw_rec(binfo->vram, binfo->width, COL8_008484, 0, 16, 15, 31);
            putstring8(binfo->vram, binfo->width, 0, 16, COL8_FFFFFF, s);
        } else if (!fifo_empty(&mousefifo)) {
            int i = fifo_get(&mousefifo);
            io_sti();

            if (mouse_decode(&mdec, i) == 1) {
                char s[40];
                sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                if ((mdec.btn & 0x01) != 0) {
                    s[1] = 'L';
                }
                if ((mdec.btn & 0x02) != 0) {
                    s[3] = 'R';
                }
                if ((mdec.btn & 0x04) != 0) {
                    s[2] = 'C';
                }
                draw_rec(binfo->vram, binfo->width, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                putstring8(binfo->vram, binfo->width, 32, 16, COL8_FFFFFF, s);

                // hidden mouse
                draw_rec(binfo->vram, binfo->width, COL8_008484, mx, my, mx + 15, my + 15);

                mx += mdec.x;
                my += mdec.y;

                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->width - 16) {
                    mx = binfo->width - 16;
                }
                if (my > binfo->height - 16) {
                    my = binfo->height - 16;
                }
                
                // draw mouse
                putblock8_8(binfo->vram, binfo->width, 16, 16, mx, my, mcursor, 16);
            }
        } else {
            io_stihlt();
        }
    }
}

