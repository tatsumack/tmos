#include "bootpack.h"

Timer* mt_timer;
int mt_tr;

void mt_init(void) {
    mt_timer = timer_alloc();
    timer_settime(mt_timer, 2);
    mt_tr = 3;
}

void mt_taskswitch(void) {
    if (mt_tr == 3) {
        mt_tr = 4;
    } else {
        mt_tr = 3;
    }
    timer_settime(mt_timer, 2);
    far_jmp(0, mt_tr * 8);
}