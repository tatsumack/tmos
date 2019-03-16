#include "bootpack.h"

FIFO keyfifo;
uchar fifobuf[KEY_FIFOBUF];

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

    fifo_init(&keyfifo, 32, fifobuf);
}
