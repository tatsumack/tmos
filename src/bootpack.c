#include "bootpack.h"
#include <stdio.h>

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

int cursor_x = 8;
int cursor_c = COL8_FFFFFF;

void init(void);

void init_mt(void);

void activate(void);

void update(void);

void update_keyboard(int val);

void update_mouse(int val);

void update_timer(int val);

void task_b_main(void);

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

    init_mt();
}

void init_mt(void) {
    task_init(memman);
    Task* task_b = task_alloc();
    task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
    task_b->tss.eip = (int)&task_b_main;
    task_b->tss.es = 1 * 8;
    task_b->tss.cs = 2 * 8;
    task_b->tss.ss = 1 * 8;
    task_b->tss.ds = 1 * 8;
    task_b->tss.fs = 1 * 8;
    task_b->tss.gs = 1 * 8;
    task_run(task_b);
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
            TMOS_ERROR("failed to allocate sht_win");
        }
        uchar* buf_win = (uchar*)memman_alloc_4k(memman, 160 * 52);
        if (!buf_win) {
            TMOS_ERROR("failed to allocate buf_win");
        }
        sheet_set_buf(sht_win, buf_win, 160, 52, -1);
        make_window(buf_win, 160, 52, "window");
        make_textbox(sht_win, 8, 28, 144, 16, COL8_FFFFFF);

        sheet_slide(sht_win, 80, 72);
        sheet_updown(sht_win, 1);
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
    io_cli();
    if (fifo_empty(&fifo)) {
        io_stihlt();
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
    sprintf(s, "%02d", val);
    sheet_putstring(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 4);
    if (get_key(val) != 0 && cursor_x < 144) {
        s[0] = get_key(val);
        s[1] = 0;
        sheet_putstring(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
        cursor_x += 8;
    }
    if (val == 0x0e && cursor_x > 8) {
        sheet_putstring(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
        cursor_x -= 8;
    }

    draw_rec(sht_win->buf, sht_win->width, cursor_c, cursor_x, 28, cursor_x + 7, 43);
    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
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

    if (mdec.btn & 0x01) {
        sheet_slide(sht_win, minfo.x - 8, minfo.y - 8);
    }
}

void update_timer(int val) {
    if (val == 10) {
        sheet_putstring(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10 sec", 6);
    }
    if (val == 3) {
        sheet_putstring(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, " 3 sec", 6);
    }
    if (val == 0 || val == 1) {
        timer_init(timer_cursor, &fifo, val ^ 1);
        timer_settime(timer_cursor, 50);

        cursor_c = val ? COL8_FFFFFF : COL8_000000;

        draw_rec(sht_win->buf, sht_win->width, cursor_c, cursor_x, 28, cursor_x + 7, 43);
        sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
    }
}

void task_b_main() {
    FIFOData fifobuf[128];
    int count = 0, count0 = 0;
    char s[12];

    FIFO fifo;
    fifo_init(&fifo, 128, fifobuf);

    Timer* timer_put = timer_alloc();
    timer_init(timer_put, &fifo, 1);
    timer_settime(timer_put, 1);

    Timer* timer_1s = timer_alloc();
    timer_init(timer_1s, &fifo, 100);
    timer_settime(timer_1s, 100);

    for (;;) {
        count++;
        io_cli();
        if (fifo_empty(&fifo)) {
            io_stihlt();
        } else {
            FIFOData data = fifo_get(&fifo);
            io_sti();
            if (data.val == 1) {
                sprintf(s, "%11d", count);
                sheet_putstring(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 11);
                timer_settime(timer_put, 1);
            } else if (data.val == 100) {
                sprintf(s, "%11d", count - count0);
                sheet_putstring(sht_back, 0, 128, COL8_FFFFFF, COL8_008484, s, 11);
                count0 = count;
                timer_settime(timer_1s, 100);
            }
        }
    }
}
