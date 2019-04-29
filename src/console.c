#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern MemoryManager* memman;

void console_task(Sheet* sht) {
    Task* task = task_now();
    FIFOData fifobuf[128];
    fifo_init(&task->fifo, 128, fifobuf, task);

    Timer* timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);

    int cursor_c = -1;
    int cursor_x = 24;
    int cursor_y = 28;

    sheet_putstring(sht, 8, 28, COL8_FFFFFF, COL8_000000, "$ ", 2);

    char s[40], cmdline[30];
    for (;;) {
        io_cli();
        if (fifo_empty(&task->fifo)) {
            task_sleep(task);
            io_stihlt();
        } else {
            FIFOData data = fifo_get(&task->fifo);
            io_sti();

            int val = data.val;

            if (data.type == fifotype_timer) {
                if (val == 1) {
                    timer_init(timer, &task->fifo, 0);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, &task->fifo, 1);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (data.type == fifotype_keyboard) {
                if (get_key(val) != 0 && cursor_x < 240) {
                    s[0] = get_key(val);
                    s[1] = 0;
                    cmdline[cursor_x / 8 - 3] = get_key(val);
                    sheet_putstring(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                    cursor_x += 8;
                }
                if (val == 0x0e && cursor_x > 24) {
                    sheet_putstring(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                    cursor_x -= 8;
                }
                if (val == 0x1c) {
                    sheet_putstring(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                    cmdline[cursor_x / 8 - 3] = 0;
                    cursor_y = cons_newline(cursor_y, sht);
                    if (strcmp(cmdline, "mem") == 0) {
                        uint memtotal = memtest(0x00400000, 0xbfffffff);
                        sprintf(s, "total %dMB", memtotal / (1024 * 1024));
                        sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sht);
                        sprintf(s, "free %dKB", memman_total_free_size(memman) / 1024);
                        sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sht);
                        cursor_y = cons_newline(cursor_y, sht);
                    } else if (strcmp(cmdline, "clear") == 0) {
                        for (int y = 28; y < 28 + 128; y++) {
                            for (int x = 8; x < 8 + 240; x++) {
                                sht->buf[x + y * sht->width] = COL8_000000;
                            }
                        }
                        sheet_refresh(sht, 8, 28, 8 + 240, 28 + 120);
                        cursor_y = 28;
                    } else if (cmdline[0] != 0) {
                        sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "command not found", 17);
                        cursor_y = cons_newline(cursor_y, sht);
                        cursor_y = cons_newline(cursor_y, sht);
                    }

                    sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "$ ", 2);
                    cursor_x = 24;
                }
                if (val == 0x0f) {
                    cursor_c = cursor_c >= 0 ? -1 : COL8_FFFFFF;
                    if (cursor_c == -1) {
                        draw_rec(sht->buf, sht->width, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
                    }
                }
            }
            if (cursor_c >= 0) {
                draw_rec(sht->buf, sht->width, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
            }
            sheet_refresh(sht, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
        }
    }
}

int cons_newline(int cursor_y, Sheet* sht) {
    if (cursor_y < 28 + 112) {
        cursor_y += 16;
    } else {
        // scroll
        for (int y = 28; y < 28 + 112; y++) {
            for (int x = 8; x < 8 + 240; x++) {
                sht->buf[x + y * sht->width] = sht->buf[x + (y + 16) * sht->width];
            }
        }
        for (int y = 28 + 112; y < 28 + 128; y++) {
            for (int x = 8; x < 8 + 240; x++) {
                sht->buf[x + y * sht->width] = COL8_000000;
            }
        }
        sheet_refresh(sht, 8, 28, 8 + 240, 28 + 128);
    }

    return cursor_y;
}
