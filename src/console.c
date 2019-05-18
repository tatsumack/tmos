#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern MemoryManager* memman;

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
    *((int*)0x0fec) = (int)&cons;

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
    } else if (cmdline[0] != 0) {
        if (cmd_app(cons, cmdline) == 0) {
            cons_putstr0(cons, "command not found\n\n");
            cons_newline(cons);
            cons_newline(cons);
        }
    }
}

void cmd_mem(Console* cons) {
    uint memtotal = memtest(0x00400000, 0xbfffffff);

    char s[60];
    sprintf(s, "total %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total_free_size(memman) / 1024);
    cons_putstr0(cons, s);
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
        cons_putstr0(cons, s);
        cons_newline(cons);
    }
    cons_newline(cons);
}

void cmd_cat(Console* cons, char* cmdline) {
    FileInfo* finfo = file_search(cmdline + 4, (FileInfo*)(ADR_DISKIMG + 0x002600), 224);

    if (finfo != 0) {
        char* p = (char*)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
        cons_putstrn(cons, p, finfo->size);
        memman_free_4k(memman, (int)p, finfo->size);
    } else {
        cons_putstr0(cons, "file not found\n");
        cons_newline(cons);
    }
    cons_newline(cons);
}

int cmd_app(Console* cons, char* cmdline) {
    char name[18];
    int i = 0;
    for (i = 0; i < 13; i++) {
        if (cmdline[i] <= ' ') break;
        name[i] = cmdline[i];
    }
    name[i] = 0;

    Task* task = task_now();

    FileInfo* finfo = file_search(name, (FileInfo*)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0 && name[i - 1] != '.') {
        name[i] = '.';
        name[i + 1] = 'b';
        name[i + 2] = 'i';
        name[i + 3] = 'n';
        name[i + 4] = 0;
        finfo = file_search(name, (FileInfo*)(ADR_DISKIMG + 0x002600), 224);
    }
    if (finfo != 0) {
        char* p = (char*)memman_alloc_4k(memman, finfo->size);
        char* q = (char*)memman_alloc_4k(memman, 64 * 1024);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
        *((int*)0xfe8) = (int)p;

        SegmentDescriptor* gdt = (SegmentDescriptor*)ADR_GDT;
        set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
        set_segmdesc(gdt + 1004, 64 * 1024 - 1, (int)q, AR_DATA32_RW + 0x60);

        if (finfo->size >= 8 && strncmp(p + 4, "TMOS", 4) == 0) {
            start_app(0x1b, 1003 * 8, 64 * 1024, 1004 * 8, &task->tss.esp0);
        } else {
            start_app(0, 1003 * 8, 64 * 1024, 1004 * 8, &task->tss.esp0);
        }

        memman_free_4k(memman, (int)p, finfo->size);
        memman_free_4k(memman, (int)q, 64 * 1024);
        cons_newline(cons);
        return 1;
    }

    return 0;
}

void cons_putstr0(Console* cons, char* s) {
    for (; *s != 0; s++) {
        cons_putchar(cons, *s, 1);
    }
}

void cons_putstrn(Console* cons, char* s, int n) {
    for (int i = 0; i < n; i++) {
        cons_putchar(cons, s[i], 1);
    }
}

int tmos_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
    Console* cons = (Console*)*(int*)0x0fec;
    int cs_base = *((int*)0xfe8);
    Task* task = task_now();
    switch (edx) {
        case 1:
            cons_putchar(cons, eax & 0xff, 1);
            break;
        case 2:
            cons_putstr0(cons, (char*)ebx + cs_base);
            break;
        case 3:
            cons_putstrn(cons, (char*)ebx + cs_base, ecx);
            break;
        case 4:
            return (int)&(task->tss.esp0);
        default:
            TMOS_ERROR("tmos_api: invalid edx");
            break;
    }
    return 0;
}

// interrupted by stack exception
int inthandler0c(int* esp) {
    Console* cons = (Console*)*(int*)0x0fec;
    Task* task = task_now();
    cons_putstr0(cons, "\nINT 0C:\n Stack Exception.\n");

    char s[30];
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return (int)&(task->tss.esp0);
}

// interrupted by general protected exception
int inthandler0d(int* esp) {
    Console* cons = (Console*)*(int*)0x0fec;
    Task* task = task_now();
    cons_putstr0(cons, "\nINT 0D:\n General Protected Exception.\n");

    char s[30];
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);

    return (int)&(task->tss.esp0);
}
