#include <stdio.h>

#include "bootpack.h"

extern FIFO keyfifo;
extern FIFO mousefifo;

void tmos_main(void) {
    BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;

    init_gdtidt();

    init_pic();

    init_keyboard();

    init_palette();

    init_screen(binfo->vram, binfo->width, binfo->height);

    MouseInfo minfo;
    MouseDec mdec;
    init_mouse(&minfo, &mdec);

    // memory
    {
        uint memtotal = memtest(0x00400000, 0xbfffffff);

        MemoryManager* memman = (MemoryManager*) ADR_MEMMAN;
        memman_init(memman);
        memman_free(memman, 0x00001000, 0x0009e000);
        memman_free(memman, 0x00400000, memtotal - 0x00400000);

        char membuf[40];
        sprintf(membuf, "memory_total: %dMB  free_total: %dKB", memtotal / (1024 * 1024),
                memman_total_free_size(memman) / 1024);
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
                draw_rec(binfo->vram, binfo->width, COL8_008484, minfo.x, minfo.y, minfo.x + 15, minfo.y + 15);

                mouse_move(&minfo, mdec.x, mdec.y);

                // draw mouse
                putblock8_8(binfo->vram, binfo->width, 16, 16, minfo.x, minfo.y, minfo.image, 16);
            }
        } else {
            io_stihlt();
        }
    }
}

