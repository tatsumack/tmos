#include "bootpack.h"

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

char get_key(int code) {
    if (code >= 0x54) return 0;

    static char keytable[0x54] = {0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,   'Q', 'W', 'E', 'R', 'T',
                                  'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,
                                  0,   ']', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,
                                  0,   0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'};
    return keytable[code];
}
