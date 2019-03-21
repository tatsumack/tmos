#include <stdio.h>
#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMERBUF_SIZE 8

#define TIMER_FLAGS_NOTUSED 0
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

extern FIFO fifo;
extern Timer* task_timer;

TimerManager timerman;
Timer* timer_cursor;

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);

    for (int i = 0; i < MAX_TIMERS; i++) {
        timerman.timers[i].flags = TIMER_FLAGS_NOTUSED;
    }

    Timer* last = timer_alloc();
    last->timeout = 0xffffffff;
    last->flags = TIMER_FLAGS_USING;
    last->next = NULL;
    timerman.front = last;
    timerman.count = 0;
}

void init_timer(void) {
    Timer* timer10 = timer_alloc();
    timer_init(timer10, &fifo, 10);
    timer_settime(timer10, 1000);

    Timer* timer3 = timer_alloc();
    timer_init(timer3, &fifo, 3);
    timer_settime(timer3, 300);

    timer_cursor = timer_alloc();
    timer_init(timer_cursor, &fifo, 1);
    timer_settime(timer_cursor, 50);
}

Timer* timer_alloc(void) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timerman.timers[i].flags != TIMER_FLAGS_NOTUSED) continue;

        timerman.timers[i].flags = TIMER_FLAGS_ALLOC;
        return &timerman.timers[i];
    }
    TMOS_ERROR("can't find not used timer");
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

    char ts = 0;

    Timer* timer = timerman.front;
    if (timerman.count < timer->timeout) {
        return;
    }

    for (;;) {
        if (timerman.count < timer->timeout) {
            break;
        }
        timer->flags = TIMER_FLAGS_ALLOC;
        if (timer == task_timer) {
            ts = 1;
        } else {
            FIFOData data;
            data.type = fifotype_timer;
            data.val = timer->data;
            fifo_put(timer->fifo, data);
        }

        timer = timer->next;
    }

    timerman.front = timer;

    if (ts == 1) {
        task_switch();
    }
}

void timer_settime(Timer* timer, uint timeout) {
    timer->timeout = timerman.count + timeout;
    timer->flags = TIMER_FLAGS_USING;

    int eflags = io_load_eflags();
    io_cli();

    // find where the timer inserts
    Timer* f = timerman.front;
    if (timer->timeout <= f->timeout) {
        timerman.front = timer;
        timer->next = f;
        io_store_eflags(eflags);
        return;
    }

    Timer* prev = f;
    Timer* cur = prev;
    for (;;) {
        prev = cur;
        cur = cur->next;
        if (timer->timeout <= cur->timeout) {
            prev->next = timer;
            timer->next = cur;
            io_store_eflags(eflags);
            return;
        }
    }
}
