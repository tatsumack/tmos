#include <stdio.h>

#include "bootpack.h"

extern FIFO keyfifo;
extern FIFO mousefifo;

MouseInfo minfo;
MouseDec mdec;
BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;
MemoryManager* memman = (MemoryManager*) ADR_MEMMAN;

SheetManager* shtman;
Sheet* sht_back;
Sheet* sht_mouse;

void init(void);

void activate(void);

void update(void);

void tmos_main(void) {
    init();

    activate();

    for (;;) {
        update();
    }
}

void init(void) {
    init_gdtidt();

    init_pic();

    init_keyboard();

    init_palette();

    init_mouse(&minfo, &mdec);

    init_memory();
}

void activate(void) {

    shtman = shtman_init(memman, binfo->vram, binfo->width, binfo->height);

    // backgrounds
    {
        sht_back = sheet_alloc(shtman);
        uchar* buf_back = (uchar*) memman_alloc_4k(memman, (uint) (binfo->width * binfo->height));
        sheet_set_buf(sht_back, buf_back, binfo->width, binfo->height, -1);

        init_screen(buf_back, binfo->width, binfo->height);

        sheet_slide(shtman, sht_back, 0, 0);
        sheet_updown(shtman, sht_back, 0);
    }

    // mouse
    {
        sht_mouse = sheet_alloc(shtman);
        uchar* buf_mouse = (uchar*) memman_alloc(memman, 256);
        sheet_set_buf(sht_mouse, buf_mouse, 16, 16, 99);

        init_mouse_cursor8(buf_mouse, 99);
        minfo.x = (binfo->width - 16) / 2;
        minfo.y = (binfo->height - 28 - 16) / 2;

        sheet_slide(shtman, sht_mouse, minfo.x, minfo.y);
        sheet_updown(shtman, sht_mouse, 1);
    }

    // memory info
    {
        uint memtotal = memtest(0x00400000, 0xbfffffff);
        char membuf[40];
        sprintf(membuf, "memory_total: %dMB  free_total: %dKB", memtotal / (1024 * 1024),
                memman_total_free_size(memman) / 1024);
        putstring8(sht_back->buf, binfo->width, 4, 32, COL8_FFFFFF, membuf);
    }

    sheet_refresh(shtman, sht_back, 0, 0, binfo->width, 48);
}

void update(void) {
    io_cli();

    if (!fifo_empty(&keyfifo)) {
        int i = fifo_get(&keyfifo);
        io_sti();

        char s[40];
        sprintf(s, "%02X", i);
        draw_rec(sht_back->buf, binfo->width, COL8_008484, 0, 16, 15, 31);
        putstring8(sht_back->buf, binfo->width, 0, 16, COL8_FFFFFF, s);
        sheet_refresh(shtman, sht_back, 0, 16, 16, 32);
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
            draw_rec(sht_back->buf, binfo->width, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
            putstring8(sht_back->buf, binfo->width, 32, 16, COL8_FFFFFF, s);
            sheet_refresh(shtman, sht_back, 32, 16, 32 + 15 * 8, 32);

            mouse_move(&minfo, mdec.x, mdec.y);

            // draw mouse
            sheet_slide(shtman, sht_mouse, minfo.x, minfo.y);
        }
    } else {
        io_stihlt();
    }
}

