#include <stdio.h>

#include "bootpack.h"

extern TimerManager timerman;
extern Timer* timer_cursor;

extern FIFO fifo;

MouseInfo minfo;
MouseDec mdec;
BootInfo* binfo = (BootInfo*)ADR_BOOTINFO;
MemoryManager* memman = (MemoryManager*)ADR_MEMMAN;

SheetManager* shtman;
Sheet* sht_back;
Sheet* sht_win;
Sheet* sht_mouse;

int count = 0;
int is_counting = 0;

void init(void);

void activate(void);

void update(void);

void update_keyboard(int val);

void update_mouse(int val);

void update_timer(int val);

void tmos_main(void) {
    init();

    activate();

    for (;;) {
        update();
    }
}

void init(void) {
    init_fifo();

    init_gdtidt();

    init_pic();

    init_keyboard();

    init_palette();

    init_mouse(&minfo, &mdec);

    init_timer();

    init_memory();
}

void activate(void) {
    shtman = shtman_init(memman, binfo->vram, binfo->width, binfo->height);

    // backgrounds
    {
        sht_back = sheet_alloc(shtman);
        uchar* buf_back = (uchar*)memman_alloc_4k(memman, (uint)(binfo->width * binfo->height));
        sheet_set_buf(sht_back, buf_back, binfo->width, binfo->height, -1);

        init_screen(buf_back, binfo->width, binfo->height);

        sheet_slide(sht_back, 0, 0);
        sheet_updown(sht_back, 0);
    }

    // window
    {
        sht_win = sheet_alloc(shtman);
        if (!sht_win) {
            TMOC_ERROR("failed to allocate sht_win");
        }
        uchar* buf_win = (uchar*)memman_alloc_4k(memman, 160 * 52);
        if (!buf_win) {
            TMOC_ERROR("failed to allocate buf_win");
        }
        sheet_set_buf(sht_win, buf_win, 160, 52, -1);
        make_window(buf_win, 160, 52, "counter");
        sheet_slide(sht_win, 80, 72);
        sheet_updown(sht_win, 1);

        sheet_putstring(sht_win, 20, 28, COL8_000000, COL8_C6C6C6, "Now counting...", 15);
    }

    // mouse
    {
        sht_mouse = sheet_alloc(shtman);
        uchar* buf_mouse = (uchar*)memman_alloc(memman, 256);
        sheet_set_buf(sht_mouse, buf_mouse, 16, 16, 99);

        init_mouse_cursor8(buf_mouse, 99);
        minfo.x = (binfo->width - 16) / 2;
        minfo.y = (binfo->height - 28 - 16) / 2;

        sheet_slide(sht_mouse, minfo.x, minfo.y);
        sheet_updown(sht_mouse, 2);
    }

    // memory info
    {
        uint memtotal = memtest(0x00400000, 0xbfffffff);
        char membuf[40];
        sprintf(membuf, "memory_total: %dMB  free_total: %dKB", memtotal / (1024 * 1024), memman_total_free_size(memman) / 1024);
        putstring(sht_back->buf, binfo->width, 4, 32, COL8_FFFFFF, membuf);
    }

    sheet_refresh(sht_back, 0, 0, binfo->width, 48);
}

void update(void) {
    if (is_counting) {
        count++;
    }
    char buf_counter[20];
    sprintf(buf_counter, "%010d", count);
    sheet_putstring(sht_win, 20, 28, COL8_000000, COL8_C6C6C6, buf_counter, 15);

    io_cli();
    if (fifo_empty(&fifo)) {
        io_sti();
        return;
    }

    FIFOData data = fifo_get(&fifo);
    io_sti();

    if (data.type == fifotype_keyboard) {
        update_keyboard(data.val);
    }
    if (data.type == fifotype_mouse) {
        update_mouse(data.val);
    }
    if (data.type == fifotype_timer) {
        update_timer(data.val);
    }
}

void update_keyboard(int val) {
    char s[40];
    sprintf(s, "%02X", val);
    sheet_putstring(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 4);
}

void update_mouse(int val) {
    if (mouse_decode(&mdec, val) != 1) return;

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
    sheet_putstring(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

    mouse_move(&minfo, mdec.x, mdec.y);

    sheet_slide(sht_mouse, minfo.x, minfo.y);
}

void update_timer(int val) {
    if (val == 10) {
        is_counting = 0;
        sheet_putstring(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10 sec", 6);
    }
    if (val == 3) {
        is_counting = 1;
        count = 0;
    }
    if (val == 0 || val == 1) {
        timer_init(timer_cursor, val ^ 1);
        timer_settime(timer_cursor, 50);

        draw_rec(sht_back->buf, sht_back->width, val ? COL8_FFFFFF : COL8_008484, 8, 96, 15, 111);
        sheet_refresh(sht_back, 8, 96, 16, 112);
    }
}
