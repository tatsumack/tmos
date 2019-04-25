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
Sheet* sht_cons;

int cursor_x = 8;
int cursor_c = COL8_FFFFFF;

Task* task_a;

void init(void);

void init_mt(void);

void activate(void);

void update(void);

void update_keyboard(int val);

void update_mouse(int val);

void update_timer(int val);

void console_task(Sheet* sht);

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

        Task* task = task_alloc();
        task->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
        task->tss.eip = (int)&console_task;
        task->tss.es = 1 * 8;
        task->tss.cs = 2 * 8;
        task->tss.ss = 1 * 8;
        task->tss.ds = 1 * 8;
        task->tss.fs = 1 * 8;
        task->tss.gs = 1 * 8;
        *((int*)(task->tss.esp + 4)) = (int)sht_cons;
        task_run(task, 2, 2);

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

        sheet_slide(sht_win, 64, 56);
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

    // memory info
    {
        uint memtotal = memtest(0x00400000, 0xbfffffff);
        char membuf[40];
        sprintf(membuf, "memory_total: %dMB  free_total: %dKB", memtotal / (1024 * 1024), memman_total_free_size(memman) / 1024);
        sheet_putstring(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, membuf, 40);
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
    char s[40];
    sprintf(s, "%02d", val);
    sheet_putstring(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 4);
    if (get_key(val) != 0 && cursor_x < 128) {
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

void console_task(Sheet* sht) {
    Task* task = task_now();
    FIFOData fifobuf[128];
    FIFO fifo;
    fifo_init(&fifo, 128, fifobuf, task);

    Timer* timer = timer_alloc();
    timer_init(timer, &fifo, 1);
    timer_settime(timer, 50);

    int cursor_c = COL8_000000;
    int cursor_x = 8;

    for (;;) {
        io_cli();
        if (fifo_empty(&fifo)) {
            task_sleep(task);
            io_stihlt();
        } else {
            FIFOData data = fifo_get(&fifo);
            io_sti();

            int val = data.val;

            if (val <= 1) {
                if (val == 1) {
                    timer_init(timer, &fifo, 0);
                    cursor_c = COL8_FFFFFF;
                } else {
                    timer_init(timer, &fifo, 1);
                    cursor_c = COL8_000000;
                }
                TMOS_DEBUG("cusor_c %d", cursor_c);
                timer_settime(timer, 50);
                draw_rec(sht->buf, sht->width, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                sheet_refresh(sht, cursor_x, 28, cursor_x + 8, 44);
            }
        }
    }
}
