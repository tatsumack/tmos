#include "bootpack.h"
#include "stdio.h"

#define FLAGS_OVERRUN 0x001
#define FIFO_SIZE 128

FIFO fifo;
FIFOData fifobuf[FIFO_SIZE];

void init_fifo(void) { fifo_init(&fifo, FIFO_SIZE, fifobuf, 0); }

void fifo_init(FIFO* fifo, int size, FIFOData* buf, Task* task) {
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->write = 0;
    fifo->read = 0;
    fifo->task = task;
}

int fifo_put(FIFO* fifo, FIFOData data) {
    if (fifo->free == 0) {
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }

    fifo->buf[fifo->write] = data;
    fifo->write++;
    if (fifo->write == fifo->size) {
        fifo->write = 0;
    }
    fifo->free--;

    if (fifo->task) {
        if (fifo->task->status != taskstatus_running) {
            task_run(fifo->task, -1, 0);
        }
    }
    return 0;
}

FIFOData fifo_get(FIFO* fifo) {
    if (fifo_empty(fifo)) {
        TMOS_ERROR("FIFO is empty");
    }

    FIFOData data = fifo->buf[fifo->read];
    fifo->read++;
    if (fifo->read == fifo->size) {
        fifo->read = 0;
    }
    fifo->free++;
    return data;
}

int fifo_empty(FIFO* fifo) { return fifo->size == fifo->free; }
