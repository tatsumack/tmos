#include "bootpack.h"
#include "stdio.h"

#define SHEET_USE 1

SheetManager* shtman_init(MemoryManager* memman, char* vram, int width, int height) {
    SheetManager* shtman = (SheetManager*)memman_alloc_4k(memman, sizeof(SheetManager));
    if (!shtman) {
        TMOS_ERROR("failed to allocate SheetManager");
        return NULL;
    }

    shtman->map = (char*)memman_alloc_4k(memman, width * height);
    if (!shtman->map) {
        TMOS_ERROR("failed to allocate sht->map");
        return NULL;
    }

    shtman->vram = vram;
    shtman->width = width;
    shtman->height = height;
    shtman->top = -1;
    for (int i = 0; i < MAX_SHEETS; i++) {
        shtman->sheets[i].flags = 0;
    }

    return shtman;
}

Sheet* sheet_alloc(SheetManager* shtman) {
    for (int i = 0; i < MAX_SHEETS; i++) {
        if (shtman->sheets[i].flags == SHEET_USE) {
            continue;
        }

        Sheet* sht = &shtman->sheets[i];
        sht->flags = SHEET_USE;
        sht->zorder = -1;
        sht->man = shtman;
        return sht;
    }

    return NULL;
}

void sheet_set_buf(Sheet* sht, uchar* buf, int width, int height, int col_inv) {
    sht->buf = buf;
    sht->width = width;
    sht->height = height;
    sht->col_inv = col_inv;
}

void sheet_updown(Sheet* sht, int zorder) {
    SheetManager* shtman = sht->man;
    int old = sht->zorder;

    // clamp
    if (zorder > shtman->top + 1) {
        zorder = shtman->top + 1;
    }

    if (zorder < -1) {
        zorder = -1;
    }

    // if the value and previous one are same, do nothing.
    if (old == zorder) {
        return;
    }

    sht->zorder = zorder;

    if (zorder < 0) {
        // invisible
        for (int i = old; i < shtman->top; i++) {
            shtman->zorders[i] = shtman->zorders[i + 1];
            shtman->zorders[i]->zorder = i;
        }
        shtman->top--;

        sheet_refresh_map(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, 0);
        sheet_refresh_sub(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, 0, old - 1);
    } else if (old < 0 && zorder >= 0) {
        // visible
        for (int i = shtman->top; i >= zorder; i++) {
            shtman->zorders[i + 1] = shtman->zorders[i];
            shtman->zorders[i + 1]->zorder = i + 1;
        }
        shtman->zorders[zorder] = sht;
        shtman->top++;

        sheet_refresh_map(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder);
        sheet_refresh_sub(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder, sht->zorder);
    } else {
        // slide
        if (old > zorder) {
            for (int i = old; i > zorder; i--) {
                shtman->zorders[i] = shtman->zorders[i - 1];
                shtman->zorders[i]->zorder = i;
            }
            shtman->zorders[zorder] = sht;

            sheet_refresh_map(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, 0);
            sheet_refresh_sub(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, 0, old - 1);
        } else {
            for (int i = old; i < zorder; i++) {
                shtman->zorders[i] = shtman->zorders[i + 1];
                shtman->zorders[i]->zorder = i;
            }
            shtman->zorders[zorder] = sht;

            sheet_refresh_map(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder);
            sheet_refresh_sub(shtman, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder, sht->zorder);
        }
    }
}

void sheet_refresh(Sheet* sht, int bx0, int by0, int bx1, int by1) {
    if (sht->height == -1) return;
    sheet_refresh_sub(sht->man, sht->vx + bx0, sht->vy + by0, sht->vx + bx1, sht->vy + by1, sht->zorder, sht->zorder);
}

void sheet_refresh_sub(SheetManager* shtman, int vx0, int vy0, int vx1, int vy1, int zorder0, int zorder1) {
    vx0 = max(vx0, 0);
    vy0 = max(vy0, 0);
    vx1 = min(vx1, shtman->width);
    vy1 = min(vy1, shtman->height);

    for (int i = max(zorder0, 0); i <= min(zorder1, shtman->top); i++) {
        Sheet* sht = shtman->zorders[i];
        uchar sid = (uchar)(sht - shtman->sheets);

        int bx0 = max(vx0 - sht->vx, 0);
        int by0 = max(vy0 - sht->vy, 0);
        int bx1 = min(vx1 - sht->vx, sht->width);
        int by1 = min(vy1 - sht->vy, sht->height);

        for (int by = by0; by < by1; by++) {
            int vy = sht->vy + by;

            for (int bx = bx0; bx < bx1; bx++) {
                int vx = sht->vx + bx;
                if (shtman->map[vy * shtman->width + vx] == sid) {
                    uchar c = sht->buf[by * sht->width + bx];
                    shtman->vram[vy * shtman->width + vx] = c;
                }
            }
        }
    }
}

void sheet_refresh_map(SheetManager* shtman, int vx0, int vy0, int vx1, int vy1, int zorder) {
    vx0 = max(vx0, 0);
    vy0 = max(vy0, 0);
    vx1 = min(vx1, shtman->width);
    vy1 = min(vy1, shtman->height);

    for (int i = max(zorder, 0); i <= shtman->top; i++) {
        Sheet* sht = shtman->zorders[i];
        uchar sid = (uchar)(sht - shtman->sheets);

        int bx0 = max(vx0 - sht->vx, 0);
        int by0 = max(vy0 - sht->vy, 0);
        int bx1 = min(vx1 - sht->vx, sht->width);
        int by1 = min(vy1 - sht->vy, sht->height);

        for (int by = by0; by < by1; by++) {
            int vy = sht->vy + by;

            for (int bx = bx0; bx < bx1; bx++) {
                int vx = sht->vx + bx;

                if (sht->buf[by * sht->width + bx] != sht->col_inv) {
                    shtman->map[vy * shtman->width + vx] = sid;
                }
            }
        }
    }
}

void sheet_slide(Sheet* sht, int vx, int vy) {
    int oldx = sht->vx;
    int oldy = sht->vy;
    sht->vx = vx;
    sht->vy = vy;
    if (sht->zorder != -1) {
        sheet_refresh_map(sht->man, oldx, oldy, oldx + sht->width, oldy + sht->height, 0);
        sheet_refresh_map(sht->man, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder);

        sheet_refresh_sub(sht->man, oldx, oldy, oldx + sht->width, oldy + sht->height, 0, sht->zorder - 1);
        sheet_refresh_sub(sht->man, sht->vx, sht->vy, sht->vx + sht->width, sht->vy + sht->height, sht->zorder, sht->zorder);
    }
}

void sheet_free(Sheet* sht) {
    if (sht->zorder != -1) {
        sheet_updown(sht, -1);
    }
    sht->flags = 0;
    sht->man = NULL;
}

void sheet_putstring(Sheet* sht, int x, int y, int c, int bg, char* s, int len) {
    draw_rec(sht->buf, sht->width, bg, x, y, x + len * 8 - 1, y + 15);
    putstring(sht->buf, sht->width, x, y, c, s);
    sheet_refresh(sht, x, y, x + len * 8, y + 16);
}
