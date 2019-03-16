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
    timerman.next_timeout = 0xffffffff;
    timerman.using = 0;
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

    if (timerman.count < timerman.next_timeout) {
        return;
    }

    int num = 0;
    for (int i = 0; i < timerman.using; i++) {
        if (timerman.orders[i]->timeout > timerman.count) {
            break;
        }
        timerman.orders[i]->flags = TIMER_FLAGS_ALLOC;
        fifo_put(timerman.orders[i]->fifo, timerman.orders[i]->data);
        num++;
    }

    timerman.using -= num;

    for (int j = 0; j < timerman.using; j++) {
        timerman.orders[j] = timerman.orders[num + j];
    }

    timerman.next_timeout = timerman.using > 0 ? timerman.orders[0]->timeout : 0xffffffff;
}

void timer_settime(Timer* timer, uint timeout) {
    timer->timeout = timerman.count + timeout;
    timer->flags = TIMER_FLAGS_USING;

    int eflags = io_load_eflags();
    io_cli();

    // find where the timer inserts
    int i;
    for (i = 0; i < timerman.using; i++) {
        if (timerman.orders[i]->timeout >= timer->timeout) {
            break;
        }
    }

    for (int j = timerman.using; j > i; j--) {
        timerman.orders[j] = timerman.orders[j - 1];
    }
    timerman.using ++;
    timerman.orders[i] = timer;
    timerman.next_timeout = timerman.orders[0]->timeout;

    io_store_eflags(eflags);
}
