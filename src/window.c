#include "bootpack.h"

void make_window(uchar* buf, int width, int height, char* title, char is_active) {
    draw_rec(buf, width, COL8_C6C6C6, 0, 0, width - 1, 0);
    draw_rec(buf, width, COL8_FFFFFF, 1, 1, width - 2, 1);
    draw_rec(buf, width, COL8_C6C6C6, 0, 0, 0, height - 1);
    draw_rec(buf, width, COL8_FFFFFF, 1, 1, 1, height - 2);
    draw_rec(buf, width, COL8_848484, width - 2, 1, width - 2, height - 2);
    draw_rec(buf, width, COL8_000000, width - 1, 0, width - 1, height - 1);
    draw_rec(buf, width, COL8_C6C6C6, 2, 2, width - 3, height - 3);
    draw_rec(buf, width, COL8_848484, 1, height - 2, width - 2, height - 2);
    draw_rec(buf, width, COL8_000000, 0, height - 1, width - 1, height - 1);
    make_wtitle(buf, width, title, is_active);
}

void make_wtitle(uchar* buf, int width, char* title, char is_active) {
    static char closebtn[14][16] = {"OOOOOOOOOOOOOOO@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "OQQQ@@QQQQ@@QQ$@", "OQQQQ@@QQ@@QQQ$@",
                                    "OQQQQQ@@@@QQQQ$@", "OQQQQQQ@@QQQQQ$@", "OQQQQQ@@@@QQQQ$@", "OQQQQ@@QQ@@QQQ$@", "OQQQ@@QQQQ@@QQ$@",
                                    "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "O$$$$$$$$$$$$$$@", "@@@@@@@@@@@@@@@@"};

    char tc, tbc;
    if (is_active) {
        tc = COL8_FFFFFF;
        tbc = COL8_000084;
    } else {
        tc = COL8_C6C6C6;
        tbc = COL8_848484;
    }
    draw_rec(buf, width, tbc, 3, 3, width - 4, 20);
    putstring(buf, width, 24, 4, tc, title);
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 16; x++) {
            char c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_848484;
            } else if (c == 'Q') {
                c = COL8_C6C6C6;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * width + (width - 21 + x)] = c;
        }
    }
}

void make_textbox(Sheet* sht, int x0, int y0, int sx, int sy, int c) {
    int x1 = x0 + sx, y1 = y0 + sy;
    draw_rec(sht->buf, sht->width, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    draw_rec(sht->buf, sht->width, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    draw_rec(sht->buf, sht->width, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    draw_rec(sht->buf, sht->width, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    draw_rec(sht->buf, sht->width, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
    draw_rec(sht->buf, sht->width, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
    draw_rec(sht->buf, sht->width, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
    draw_rec(sht->buf, sht->width, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    draw_rec(sht->buf, sht->width, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
}