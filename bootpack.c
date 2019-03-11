// asmfunc
void io_hlt(void);

void io_cli(void);

void io_out8(int port, int data);

int io_load_eflags(void);

void io_store_eflags(int eflags);


// declare
void init_palette(void);

void set_palette(unsigned char* rgb, int size);

void draw_rec(unsigned char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1);

#define COL8_000000        0
#define COL8_FF0000        1
#define COL8_00FF00        2
#define COL8_FFFF00        3
#define COL8_0000FF        4
#define COL8_FF00FF        5
#define COL8_00FFFF        6
#define COL8_FFFFFF        7
#define COL8_C6C6C6        8
#define COL8_840000        9
#define COL8_008400        10
#define COL8_848400        11
#define COL8_000084        12
#define COL8_840084        13
#define COL8_008484        14
#define COL8_848484        15

typedef struct BootInfo {
    char cyles, leds, vmode, reserve;
    short width, height;
    char* vram;
} BootInfo;

void tmos_main(void) {
    init_palette();

    BootInfo* binfo = (BootInfo*) 0x0ff0;

    char* vram = (char*) binfo->vram;
    int width = binfo->width;
    int height = binfo->height;

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

    for (;;) {
        io_hlt();
    }
}

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
    return;
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
    return;
}

void draw_rec(unsigned char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++)
            vram[y * width + x] = c;
    }
    return;
}
