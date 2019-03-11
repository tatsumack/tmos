#include "bootpack.h"

#define FLAGS_OVERRUN 0x001

void fifo_init(FIFO* fifo, int size, unsigned char* buf) {
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->write = 0;
    fifo->read = 0;
}

int fifo_put(FIFO* fifo, unsigned char data) {
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

int fifo_get(FIFO* fifo) {
    if (fifo_empty(fifo)) {
        return -1;
    }

    int data = fifo->buf[fifo->read];
    fifo->read++;
    if (fifo->read == fifo->size) {
        fifo->read = 0;
    }
    fifo->free++;
    return data;
}

int fifo_empty(FIFO* fifo) {
    return fifo->size == fifo->free;
}
