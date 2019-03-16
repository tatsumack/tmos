#include <stdio.h>
#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMERBUF_SIZE 8

#define TIMER_FLAGS_NOTUSED 0
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

TimerManager timerman;

FIFO timerfifo;
char timerbuf[TIMERBUF_SIZE];

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);

    timerman.count = 0;
    for (int i = 0; i < MAX_TIMERS; i++) {
        timerman.timers[i].flags = TIMER_FLAGS_NOTUSED;
    }
}

void init_timer(void) {
    fifo_init(&timerfifo, TIMERBUF_SIZE, timerbuf);

    Timer* timer = timer_alloc();
    timer_init(timer, &timerfifo, 1);
    timer_settime(timer, 1000);
}

Timer* timer_alloc(void) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timerman.timers[i].flags != TIMER_FLAGS_NOTUSED) continue;

        timerman.timers[i].flags = TIMER_FLAGS_ALLOC;
        return &timerman.timers[i];
    }
    TMOC_ERROR("can't find not used timer");
}

void timer_free(Timer* timer) { timer->flags = TIMER_FLAGS_NOTUSED; }

void timer_init(Timer* timer, FIFO* fifo, uchar data) {
    timer->fifo = fifo;
    timer->data = data;
}

// interrupted by PIT
void inthandler20(int* esp) {
    io_out8(PIC0_OCW2, 0x60);  // ack to IRQ-00
    timerman.count++;

    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timerman.timers[i].flags != TIMER_FLAGS_USING) continue;

        timerman.timers[i].timeout--;
        if (timerman.timers[i].timeout > 0) continue;

        timerman.timers[i].flags = TIMER_FLAGS_ALLOC;
        fifo_put(timerman.timers[i].fifo, timerman.timers[i].data);
    }
}

void timer_settime(Timer* timer, uint timeout) {
    timer->timeout = timeout;
    timer->flags = TIMER_FLAGS_USING;
}
