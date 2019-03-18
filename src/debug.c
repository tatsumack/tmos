#include "bootpack.h"
#include "stdio.h"

extern BootInfo* binfo;

#define CHAR_HEIGHT 16

void tmoc_error(char* s, char* file, int line) {
    // bg
    draw_rec(binfo->vram, binfo->width, COL8_FF0000, 0, binfo->height - CHAR_HEIGHT * 2, binfo->width, binfo->height);

    // file
    char file_buf[100];
    sprintf(file_buf, "file: %s", file);
    putstring(binfo->vram, binfo->width, 0, binfo->height - CHAR_HEIGHT * 2, COL8_FFFFFF, file_buf);

    // line
    char line_buf[50];
    sprintf(line_buf, "line: %d", line);
    putstring(binfo->vram, binfo->width, 200, binfo->height - CHAR_HEIGHT * 2, COL8_FFFFFF, line_buf);

    // log
    putstring(binfo->vram, binfo->width, 0, binfo->height - CHAR_HEIGHT, COL8_FFFFFF, s);

    for (;;) {
        io_hlt();
    }
}

void tmoc_debug(char* s, char* file, int line) {
    // bg
    draw_rec(binfo->vram, binfo->width, COL8_000000, 0, binfo->height - CHAR_HEIGHT * 2, binfo->width, binfo->height);

    // file
    char file_buf[100];
    sprintf(file_buf, "file: %s", file);
    putstring(binfo->vram, binfo->width, 0, binfo->height - CHAR_HEIGHT * 2, COL8_FFFFFF, file_buf);

    // line
    char line_buf[50];
    sprintf(line_buf, "line: %d", line);
    putstring(binfo->vram, binfo->width, 200, binfo->height - CHAR_HEIGHT * 2, COL8_FFFFFF, line_buf);

    // log
    putstring(binfo->vram, binfo->width, 0, binfo->height - CHAR_HEIGHT, COL8_FFFFFF, s);
}
