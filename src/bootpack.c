#include "bootpack.h"
#include <stdio.h>
#include <string.h>

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
Sheet* sht_cons;

int cursor_x = 8;
int cursor_c = COL8_FFFFFF;

int key_to = 0;
int key_ctrl = 0;

Task* task_a;
Task* task_cons;

void init(void);

void init_mt(void);

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

    init_mt();
}

void init_mt(void) {
    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 0);
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

    // sht_cons
    {
        sht_cons = sheet_alloc(shtman);
        uchar* buf = (uchar*)memman_alloc_4k(memman, 256 * 165);
        sheet_set_buf(sht_cons, buf, 256, 165, -1);
        make_window(buf, 256, 165, "console", 0);
        make_textbox(sht_cons, 8, 28, 240, 128, COL8_000000);

        task_cons = task_alloc();
        task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
        task_cons->tss.eip = (int)&console_task;
        task_cons->tss.es = 1 * 8;
        task_cons->tss.cs = 2 * 8;
        task_cons->tss.ss = 1 * 8;
        task_cons->tss.ds = 1 * 8;
        task_cons->tss.fs = 1 * 8;
        task_cons->tss.gs = 1 * 8;
        *((int*)(task_cons->tss.esp + 4)) = (int)sht_cons;
        task_run(task_cons, 2, 2);

        sheet_slide(sht_cons, 32, 4);
        sheet_updown(sht_cons, 1);
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
        sheet_set_buf(sht_win, buf_win, 144, 52, -1);
        make_window(buf_win, 144, 52, "task_a", 1);
        make_textbox(sht_win, 8, 28, 128, 16, COL8_FFFFFF);

        sheet_slide(sht_win, 500, 560);
        sheet_updown(sht_win, 2);
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
        sheet_updown(sht_mouse, 3);
    }
}

void update(void) {
    io_cli();
    if (fifo_empty(&fifo)) {
        task_sleep(task_a);
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
    if (key_to == 1 && (get_key(val) != 0 || val == 0x0e || val == 0x1c)) {
        FIFOData data;
        data.type = fifotype_keyboard;
        data.val = val;
        fifo_put(&task_cons->fifo, data);
        return;
    }

    if (key_ctrl == 1 && val == 0xae && task_cons->tss.ss0 != 0) {
        Console* cons = (Console*)*(int*)0x0fec;
        cons_putstr0(cons, "\nquit.\n");
        io_cli();
        task_cons->tss.eax = (int)&(task_cons->tss.esp0);
        task_cons->tss.eip = (int)asm_end_app;
        return;
    }

    if (get_key(val) != 0 && cursor_x < 128) {
        char s[40];
        s[0] = get_key(val);
        s[1] = 0;
        sheet_putstring(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
        cursor_x += 8;
    }
    if (val == 0x0e && cursor_x > 8) {
        sheet_putstring(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
        cursor_x -= 8;
    }
    if (val == 0x0f) {
        if (key_to == 0) {
            key_to = 1;
            make_wtitle(sht_win->buf, sht_win->width, "task_a", 0);
            make_wtitle(sht_cons->buf, sht_cons->width, "console", 1);
            cursor_c = -1;
            draw_rec(sht_win->buf, sht_win->width, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
        } else {
            key_to = 0;
            make_wtitle(sht_win->buf, sht_win->width, "task_a", 1);
            make_wtitle(sht_cons->buf, sht_cons->width, "console", 0);
            cursor_c = COL8_000000;
        }

        FIFOData data;
        data.type = fifotype_keyboard;
        data.val = val;
        fifo_put(&task_cons->fifo, data);

        sheet_refresh(sht_win, 0, 0, sht_win->width, 21);
        sheet_refresh(sht_cons, 0, 0, sht_cons->width, 21);
    }
    if (val == 0x1c || val == 0x1d) {
        key_ctrl = 1;
    }
    if (val == 0x9c || val == 0x9d) {
        key_ctrl = 0;
    }

    if (cursor_c >= 0) {
        draw_rec(sht_win->buf, sht_win->width, cursor_c, cursor_x, 28, cursor_x + 7, 43);
    }
    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
}

void update_mouse(int val) {
    if (mouse_decode(&mdec, val) != 1) return;

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

        if (cursor_c >= 0) {
            cursor_c = val ? COL8_FFFFFF : COL8_000000;
        }

        if (cursor_c >= 0) {
            draw_rec(sht_win->buf, sht_win->width, cursor_c, cursor_x, 28, cursor_x + 7, 43);
            sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
        }
    }
}
