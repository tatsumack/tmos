#include "bootpack.h"
#include "stdio.h"

#define FLAGS_OVERRUN 0x001
#define FIFO_SIZE 128

FIFO fifo;
FIFOData fifobuf[FIFO_SIZE];

void init_fifo(void) { fifo_init(&fifo, FIFO_SIZE, fifobuf); }

void fifo_init(FIFO* fifo, int size, FIFOData* buf) {
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->write = 0;
    fifo->read = 0;
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
    return 0;
}

FIFOData fifo_get(FIFO* fifo) {
    if (fifo_empty(fifo)) {
        TMOC_ERROR("FIFO is empty");
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
