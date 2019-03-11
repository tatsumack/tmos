#include <stdio.h>

#include "bootpack.h"

typedef struct MouseDec {
    unsigned char buf[3], phase;
    int x, y, btn;
} MouseDec;

extern FIFO keyfifo;
extern FIFO mousefifo;

void enable_mouse(MouseDec* mdec);

void init_keyboard(void);

int mouse_decode(MouseDec* mdec, unsigned char dat);

void tmos_main(void) {
    init_gdtidt();
    init_pic();

    // enable interrupt
    io_sti();

    unsigned char keybuf[32];
    fifo_init(&keyfifo, 32, (unsigned char*) keybuf);

    unsigned char mousebuf[128];
    fifo_init(&mousefifo, 128, (unsigned char*) mousebuf);

    io_out8(PIC0_IMR, 0xf9); // enable keyboard interrupt and PIC1
    io_out8(PIC1_IMR, 0xef); // enable mouse interrupt

    init_keyboard();

    init_palette();

    BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;
    init_screen(binfo->vram, binfo->width, binfo->height);

    char mcursor[256];
    init_mouse_cursor8(mcursor, COL8_008484);
    int mx = (binfo->width - 16) / 2;
    int my = (binfo->height - 28 - 16) / 2;
    putblock8_8(binfo->vram, binfo->width, 16, 16, mx, my, mcursor, 16);

    putstring8(binfo->vram, binfo->width, 31, 31, COL8_000000, "TMOS");
    putstring8(binfo->vram, binfo->width, 30, 30, COL8_FFFFFF, "TMOS");

    MouseDec mdec;
    enable_mouse(&mdec);

    for (;;) {
        io_cli();
        if (!fifo_empty(&keyfifo)) {
            int i = fifo_get(&keyfifo);
            io_sti();

            char s[40];
            sprintf(s, "%02X", i);
            draw_rec(binfo->vram, binfo->width, COL8_008484, 0, 16, 15, 31);
            putstring8(binfo->vram, binfo->width, 0, 16, COL8_FFFFFF, s);
        } else if (!fifo_empty(&mousefifo)) {
            int i = fifo_get(&mousefifo);
            io_sti();

            if (mouse_decode(&mdec, i) == 1) {
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
                draw_rec(binfo->vram, binfo->width, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                putstring8(binfo->vram, binfo->width, 32, 16, COL8_FFFFFF, s);

                // hidden mouse
                draw_rec(binfo->vram, binfo->width, COL8_008484, mx, my, mx + 15, my + 15);

                mx += mdec.x;
                my += mdec.y;

                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->width - 16) {
                    mx = binfo->width - 16;
                }
                if (my > binfo->height - 16) {
                    my = binfo->height - 16;
                }
                
                // draw mouse
                putblock8_8(binfo->vram, binfo->width, 16, 16, mx, my, mcursor, 16);
            }
        } else {
            io_stihlt();
        }
    }
}

#define PORT_KEYDAT                0x0060
#define PORT_KEYSTA                0x0064
#define PORT_KEYCMD                0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE        0x60
#define KBC_MODE                0x47

void wait_kbc_sendready(void) {
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

void init_keyboard(void) {
    wait_kbc_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_kbc_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

#define KEYCMD_SENDTO_MOUSE        0xd4
#define MOUSECMD_ENABLE            0xf4

void enable_mouse(MouseDec* mdec) {
    wait_kbc_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_kbc_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    mdec->phase = 0;
}

int mouse_decode(MouseDec* mdec, unsigned char dat) {
    if (mdec->phase == 0) {
        // wait initial ack
        if (dat == 0xfa) {
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1) {
        // 1st byte
        if ((dat & 0xc8) == 0x08) { // validation
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2) {
        // 2nd byte
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3) {
        // 3rd byte
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];

        // sign
        if ((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xffffff00;
        }
        if ((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xffffff00;
        }

        mdec->y = -mdec->y;

        return 1;
    }
    return -1;
}
