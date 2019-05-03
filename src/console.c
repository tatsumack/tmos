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

    FileInfo* finfo = (FileInfo*)(ADR_DISKIMG + 0x002600);

    int* fat = (int*)memman_alloc_4k(memman, 4 * 2880);
    file_readfat(fat, (uchar*)(ADR_DISKIMG + 0x000200));

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
                    } else if (strcmp(cmdline, "ls") == 0) {
                        for (int x = 0; x < 224; x++) {
                            if (finfo[x].name[0] == 0x00) break;
                            if (finfo[x].name[0] == 0xe5) continue;
                            if ((finfo[x].type & 0x18) != 0) continue;
                            sprintf(s, "filename.ext %7d", finfo[x].size);
                            for (int y = 0; y < 8; y++) {
                                s[y] = finfo[x].name[y];
                            }
                            s[9] = finfo[x].ext[0];
                            s[10] = finfo[x].ext[1];
                            s[11] = finfo[x].ext[2];
                            sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                            cursor_y = cons_newline(cursor_y, sht);
                        }
                        cursor_y = cons_newline(cursor_y, sht);
                    } else if (strncmp(cmdline, "cat ", 4) == 0) {
                        for (int i = 0; i < 11; i++) s[i] = ' ';
                        int i = 0;
                        for (int j = 4; i < 11 && cmdline[j] != 0; j++) {
                            if (cmdline[j] == '.' && i <= 8) {
                                i = 8;
                            } else {
                                s[i] = cmdline[j];
                                if ('a' <= s[i] && s[i] <= 'z') s[i] -= 0x20;
                                i++;
                            }
                        }

                        int x = 0;
                        for (x = 0; x < 224;) {
                            if (finfo[x].name[0] == 0x00) break;
                            if ((finfo[x].type & 0x18) == 0) {
                                char found = 1;
                                for (int y = 0; y < 11; y++) {
                                    if (finfo[x].name[y] != s[y]) {
                                        found = 0;
                                        break;
                                    }
                                }
                                if (found) break;
                            }
                            x++;
                        }

                        if (x < 224 && finfo[x].name[0] != 0x00) {
                            char* p = (char*)memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
                            cursor_x = 8;
                            for (int i = 0; i < finfo[x].size; i++) {
                                s[0] = p[i];
                                s[1] = 0;
                                if (s[0] == 0x09) {
                                    // tab
                                    for (;;) {
                                        sheet_putstring(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                                        cursor_x += 8;
                                        if (cursor_x == 8 + 240) {
                                            cursor_x = 8;
                                            cursor_y = cons_newline(cursor_y, sht);
                                        }
                                        if (((cursor_x - 8) & 0x1f) == 0) {
                                            break;
                                        }
                                    }
                                } else if (s[0] == 0x0a) {
                                    // LF
                                    cursor_x = 8;
                                    cursor_y = cons_newline(cursor_y, sht);
                                } else if (s[0] == 0x0d) {
                                    // CR
                                } else {
                                    sheet_putstring(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                                    cursor_x += 8;
                                    if (cursor_x == 8 + 240) {
                                        cursor_x = 8;
                                        cursor_y = cons_newline(cursor_y, sht);
                                    }
                                }
                            }
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        } else {
                            sheet_putstring(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "file not found", 14);
                            cursor_y = cons_newline(cursor_y, sht);
                        }
                        cursor_y = cons_newline(cursor_y, sht);
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

void file_readfat(int* fat, uchar* img) {
    int j = 0;
    for (int i = 0; i < 2880; i += 2) {
        // ab cd ef -> dab efc
        fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
        fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
        j += 3;
    }
}

void file_loadfile(int clustno, int size, char* buf, int* fat, char* img) {
    for (;;) {
        if (size <= 512) {
            for (int i = 0; i < size; i++) {
                buf[i] = img[clustno * 512 + i];
            }
            break;
        }
        for (int i = 0; i < 512; i++) {
            buf[i] = img[clustno * 512 + i];
        }
        size -= 512;
        buf += 512;
        clustno = fat[clustno];
    }
}
