#include "bootpack.h"

FIFO mousefifo;
uchar fifobuf[MOUSE_FIFOBUF];

void init_mouse(MouseInfo* minfo, MouseDec* mdec) {
    BootInfo* binfo = (BootInfo*)ADR_BOOTINFO;

    wait_kbc_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_kbc_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    mdec->phase = 0;

    fifo_init(&mousefifo, 128, fifobuf);
}

int mouse_decode(MouseDec* mdec, uchar dat) {
    if (mdec->phase == 0) {
        // wait initial ack
        if (dat == 0xfa) {
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1) {
        // 1st byte
        if ((dat & 0xc8) == 0x08) {  // validation
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

void mouse_move(MouseInfo* minfo, int dx, int dy) {
    BootInfo* binfo = (BootInfo*)ADR_BOOTINFO;

    minfo->x += dx;
    minfo->y += dy;

    minfo->x = clamp(minfo->x, 0, binfo->width - 1);
    minfo->y = clamp(minfo->y, 0, binfo->height - 1);
}
