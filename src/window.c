#include "bootpack.h"

void make_window(uchar* buf, int width, int height, char* title) {
    static char closebtn[14][16] = {
            "OOOOOOOOOOOOOOO@",
            "OQQQQQQQQQQQQQ$@",
            "OQQQQQQQQQQQQQ$@",
            "OQQQ@@QQQQ@@QQ$@",
            "OQQQQ@@QQ@@QQQ$@",
            "OQQQQQ@@@@QQQQ$@",
            "OQQQQQQ@@QQQQQ$@",
            "OQQQQQ@@@@QQQQ$@",
            "OQQQQ@@QQ@@QQQ$@",
            "OQQQ@@QQQQ@@QQ$@",
            "OQQQQQQQQQQQQQ$@",
            "OQQQQQQQQQQQQQ$@",
            "O$$$$$$$$$$$$$$@",
            "@@@@@@@@@@@@@@@@"
    };
    draw_rec(buf, width, COL8_C6C6C6, 0, 0, width - 1, 0);
    draw_rec(buf, width, COL8_FFFFFF, 1, 1, width - 2, 1);
    draw_rec(buf, width, COL8_C6C6C6, 0, 0, 0, height - 1);
    draw_rec(buf, width, COL8_FFFFFF, 1, 1, 1, height - 2);
    draw_rec(buf, width, COL8_848484, width - 2, 1, width - 2, height - 2);
    draw_rec(buf, width, COL8_000000, width - 1, 0, width - 1, height - 1);
    draw_rec(buf, width, COL8_C6C6C6, 2, 2, width - 3, height - 3);
    draw_rec(buf, width, COL8_000084, 3, 3, width - 4, 20);
    draw_rec(buf, width, COL8_848484, 1, height - 2, width - 2, height - 2);
    draw_rec(buf, width, COL8_000000, 0, height - 1, width - 1, height - 1);
    putstring8(buf, width, 24, 4, COL8_FFFFFF, title);
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