#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMERBUF_SIZE 8

TimerManager timerman;

FIFO timerfifo;
char timerbuf[TIMERBUF_SIZE];

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);

    timerman.count = 0;
    timerman.timeout = 0;
}

void init_timer(void) { fifo_init(&timerfifo, TIMERBUF_SIZE, timerbuf); }

// interrupted by PIT
void inthandler20(int* esp) {
    io_out8(PIC0_OCW2, 0x60);  // ack to IRQ-00
    timerman.count++;

    if (timerman.timeout > 0) {
        timerman.timeout--;
        if (timerman.timeout == 0) {
            fifo_put(timerman.fifo, timerman.data);
        }
    }
}

void settimer(uint timeout, FIFO* fifo, uchar data) {
    int eflags = io_load_eflags();
    io_cli();
    timerman.timeout = timeout;
    timerman.fifo = fifo;
    timerman.data = data;
    io_store_eflags(eflags);
}
