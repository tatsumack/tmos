#include "bootpack.h"

void init_palette(void) {
    static unsigned char table_rgb[16 * 3] = {
            0x00, 0x00, 0x00,    /*  0:Black */
            0xff, 0x00, 0x00,    /*  1:Red */
            0x00, 0xff, 0x00,    /*  2:Green */
            0xff, 0xff, 0x00,    /*  3:Yellow */
            0x00, 0x00, 0xff,    /*  4:Blue */
            0xff, 0x00, 0xff,    /*  5:Purple */
            0x00, 0xff, 0xff,    /*  6:Cyan */
            0xff, 0xff, 0xff,    /*  7:White */
            0xc6, 0xc6, 0xc6,    /*  8:Gray */
            0x84, 0x00, 0x00,    /*  9:Dark Red */
            0x00, 0x84, 0x00,    /* 10:Dark Green */
            0x84, 0x84, 0x00,    /* 11:Dark Yellow */
            0x00, 0x00, 0x84,    /* 12:Dark Blue */
            0x84, 0x00, 0x84,    /* 13:Dark Purple */
            0x00, 0x84, 0x84,    /* 14:Dark Cyan */
            0x84, 0x84, 0x84     /* 15:Dark Gray */
    };
    set_palette(table_rgb, 16);
}

void set_palette(unsigned char* rgb, int size) {
    int eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, 0);
    for (int i = 0; i < size; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);
}

void init_screen(char* vram, int width, int height) {
    draw_rec(vram, width, COL8_008484, 0, 0, width - 1, height - 29);
    draw_rec(vram, width, COL8_C6C6C6, 0, height - 28, width - 1, height - 28);
    draw_rec(vram, width, COL8_FFFFFF, 0, height - 27, width - 1, height - 27);
    draw_rec(vram, width, COL8_C6C6C6, 0, height - 26, width - 1, height - 1);

    draw_rec(vram, width, COL8_FFFFFF, 3, height - 24, 59, height - 24);
    draw_rec(vram, width, COL8_FFFFFF, 2, height - 24, 2, height - 4);
    draw_rec(vram, width, COL8_848484, 3, height - 4, 59, height - 4);
    draw_rec(vram, width, COL8_848484, 59, height - 23, 59, height - 5);
    draw_rec(vram, width, COL8_000000, 2, height - 3, 59, height - 3);
    draw_rec(vram, width, COL8_000000, 60, height - 24, 60, height - 3);

    draw_rec(vram, width, COL8_848484, width - 47, height - 24, width - 4, height - 24);
    draw_rec(vram, width, COL8_848484, width - 47, height - 23, width - 47, height - 4);
    draw_rec(vram, width, COL8_FFFFFF, width - 47, height - 3, width - 4, height - 3);
    draw_rec(vram, width, COL8_FFFFFF, width - 3, height - 24, width - 3, height - 3);
}

void draw_rec(char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++)
            vram[y * width + x] = c;
    }
}

void putfont8(char* vram, int width, int x, int y, char color, char* font) {
    // NOTE font size = 8 * 16
    for (int i = 0; i < 16; i++) {
        char* p = vram + (y + i) * width + x;
        char d = font[i];
        for (int j = 0; j < 8; j++) {
            if ((d >> j) & 1) {
                p[7 - j] = color;
            }
        }
    }
}

void putstring8(char* vram, int width, int x, int y, char color, unsigned char* s) {
    extern char ascii_fonts[4096];
    while (*s != 0x00) {
        putfont8(vram, width, x, y, color, ascii_fonts + *s * 16);
        x += 8;
        s++;
    }
}

void init_mouse_cursor8(char* mouse, char bc) {
    static char cursor[16][16] = {
            "**************..",
            "*OOOOOOOOOOO*...",
            "*OOOOOOOOOO*....",
            "*OOOOOOOOO*.....",
            "*OOOOOOOO*......",
            "*OOOOOOO*.......",
            "*OOOOOOO*.......",
            "*OOOOOOOO*......",
            "*OOOO**OOO*.....",
            "*OOO*..*OOO*....",
            "*OO*....*OOO*...",
            "*O*......*OOO*..",
            "**........*OOO*.",
            "*..........*OOO*",
            "............*OO*",
            ".............***"
    };
    int x, y;

    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor[y][x] == '.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
}

void putblock8_8(char* vram, int width, int pwidth, int pheight, int px0, int py0, char* buf, int bwidth) {
    for (int y = 0; y < pheight; y++) {
        for (int x = 0; x < pwidth; x++) {
            vram[(py0 + y) * width + (px0 + x)] = buf[y * bwidth + x];
        }
    }
}
