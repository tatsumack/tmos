#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern MemoryManager* memman;

typedef struct Console {
    Sheet* sht;
    int cur_x, cur_y, cur_c;
} Console;

void cons_putchar(Console* cons, int chr, char move);
void cons_newline(Console* cons);
void cons_runcmd(char* cmdline, Console* cons);

void cmd_mem(Console* cons);
void cmd_clear(Console* cons);
void cmd_ls(Console* cons);
void cmd_cat(Console* cons, char* cmdline);
void cmd_hlt(Console* cons);

int* fat = NULL;

void console_task(Sheet* sht) {
    Task* task = task_now();
    FIFOData fifobuf[128];
    fifo_init(&task->fifo, 128, fifobuf, task);

    Timer* timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);

    Console cons;
    cons.sht = sht;
    cons.cur_c = -1;
    cons.cur_x = 24;
    cons.cur_y = 28;

    sheet_putstring(sht, 8, 28, COL8_FFFFFF, COL8_000000, "$ ", 2);

    fat = (int*)memman_alloc_4k(memman, 4 * 2880);
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
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, &task->fifo, 1);
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (data.type == fifotype_keyboard) {
                if (get_key(val) != 0 && cons.cur_x < 240) {
                    cmdline[cons.cur_x / 8 - 3] = get_key(val);
                    cons_putchar(&cons, get_key(val), 1);
                }
                if (val == 0x0e && cons.cur_x > 24) {
                    cons_putchar(&cons, ' ', 0);
                    cons.cur_x -= 8;
                }
                if (val == 0x1c) {
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x / 8 - 3] = 0;
                    cons_newline(&cons);
                    cons_runcmd(cmdline, &cons);
                    cons_putchar(&cons, '$', 1);
                    cons_putchar(&cons, ' ', 1);
                }
                if (val == 0x0f) {
                    cons.cur_c = cons.cur_c >= 0 ? -1 : COL8_FFFFFF;
                    if (cons.cur_c == -1) {
                        draw_rec(sht->buf, sht->width, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
                    }
                }
            }
            if (cons.cur_y >= 0) {
                draw_rec(sht->buf, sht->width, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
            }
            sheet_refresh(sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
        }
    }
}

void cons_putchar(Console* cons, int chr, char move) {
    char s[2];
    s[0] = chr;
    s[1] = 0;

    if (s[0] == 0x09) {
        // tab
        for (;;) {
            sheet_putstring(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 240) {
                cons_newline(cons);
            }
            if (((cons->cur_x - 8) & 0x1f) == 0) {
                break;
            }
        }
    } else if (s[0] == 0x0a) {
        // LF
        cons_newline(cons);
    } else if (s[0] == 0x0d) {
        // CR
    } else {
        sheet_putstring(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
        if (move > 0) {
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 240) {
                cons_newline(cons);
            }
        }
    }
}

void cons_newline(Console* cons) {
    if (cons->cur_y < 28 + 112) {
        cons->cur_y += 16;
    } else {
        // scroll
        for (int y = 28; y < 28 + 112; y++) {
            for (int x = 8; x < 8 + 240; x++) {
                cons->sht->buf[x + y * cons->sht->width] = cons->sht->buf[x + (y + 16) * cons->sht->width];
            }
        }
        for (int y = 28 + 112; y < 28 + 128; y++) {
            for (int x = 8; x < 8 + 240; x++) {
                cons->sht->buf[x + y * cons->sht->width] = COL8_000000;
            }
        }
        sheet_refresh(cons->sht, 8, 28, 8 + 240, 28 + 128);
    }
    cons->cur_x = 8;
}

void cons_runcmd(char* cmdline, Console* cons) {
    if (strcmp(cmdline, "mem") == 0) {
        cmd_mem(cons);
    } else if (strcmp(cmdline, "clear") == 0) {
        cmd_clear(cons);
    } else if (strcmp(cmdline, "ls") == 0) {
        cmd_ls(cons);
    } else if (strncmp(cmdline, "cat ", 4) == 0) {
        cmd_cat(cons, cmdline);
    } else if (strcmp(cmdline, "hlt") == 0) {
        cmd_hlt(cons);
    } else if (cmdline[0] != 0) {
        sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "command not found", 17);
        cons_newline(cons);
        cons_newline(cons);
    }
}

void cmd_mem(Console* cons) {
    uint memtotal = memtest(0x00400000, 0xbfffffff);

    char s[30];
    sprintf(s, "total %dMB", memtotal / (1024 * 1024));
    sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
    cons_newline(cons);
    sprintf(s, "free %dKB", memman_total_free_size(memman) / 1024);
    sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
    cons_newline(cons);
    cons_newline(cons);
}

void cmd_clear(Console* cons) {
    for (int y = 28; y < 28 + 128; y++) {
        for (int x = 8; x < 8 + 240; x++) {
            cons->sht->buf[x + y * cons->sht->width] = COL8_000000;
        }
    }
    sheet_refresh(cons->sht, 8, 28, 8 + 240, 28 + 120);
    cons->cur_y = 28;
}

void cmd_ls(Console* cons) {
    FileInfo* finfo = (FileInfo*)(ADR_DISKIMG + 0x002600);
    char s[30];

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
        sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
        cons_newline(cons);
    }
    cons_newline(cons);
}
void cmd_cat(Console* cons, char* cmdline) {
    FileInfo* finfo = file_search(cmdline + 4, (FileInfo*)(ADR_DISKIMG + 0x002600), 224);

    if (finfo != 0) {
        char* p = (char*)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
        for (int i = 0; i < finfo->size; i++) {
            cons_putchar(cons, p[i], 1);
        }
        memman_free_4k(memman, (int)p, finfo->size);
    } else {
        sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "file not found", 14);
        cons_newline(cons);
    }
    cons_newline(cons);
}

void cmd_hlt(Console* cons) {
    FileInfo* finfo = file_search("HLT.BIN", (FileInfo*)(ADR_DISKIMG + 0x002600), 224);
    SegmentDescriptor* gdt = (SegmentDescriptor*)ADR_GDT;

    if (finfo != 0) {
        char* p = (char*)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
        set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);
        far_jmp(0, 1003 * 8);
        memman_free_4k(memman, (int)p, finfo->size);
    } else {
        sheet_putstring(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "file not found", 14);
        cons_newline(cons);
        cons_newline(cons);
    }
    cons_newline(cons);
}
