#include "bootpack.h"

extern FIFO fifo;

void init_pic(void) {
    // disable all interrupts
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    // settings for PIC0
    io_out8(PIC0_ICW1, 0x11);
    io_out8(PIC0_ICW2, 0x20);    // IRQ0-7 map INT20-27
    io_out8(PIC0_ICW3, 1 << 2);  // connect PIC1 through IRQ2
    io_out8(PIC0_ICW4, 0x01);

    // settings for PIC1
    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2, 0x28);  // IRQ8-15 map INT28-2f
    io_out8(PIC1_ICW3, 2);     // connect PIC1 through IRQ2
    io_out8(PIC1_ICW4, 0x01);

    // enable interrupts
    io_sti();

    // enable interval timer
    init_pit();

    io_out8(PIC0_IMR, 0xf8);  // enable keyboard interrupt, PIT and PIC1
    io_out8(PIC1_IMR, 0xef);  // enable mouse interrupt
}

#define PORT_KEYDAT 0x0060

// interrupted by keyboard
void inthandler21(int* esp) {
    io_out8(PIC0_OCW2, 0x61);  // ack

    FIFOData data;
    data.type = fifotype_keyboard;
    data.val = io_in8(PORT_KEYDAT);
    fifo_put(&fifo, data);
}

// interrupted by mouse
void inthandler2c(int* esp) {
    io_out8(PIC1_OCW2, 0x64);  // ack
    io_out8(PIC0_OCW2, 0x62);  // ack

    FIFOData data;
    data.type = fifotype_mouse;
    data.val = io_in8(PORT_KEYDAT);
    fifo_put(&fifo, data);
}

// interrupted by PIC1 on initialization
void inthandler27(int* esp) {
    io_out8(PIC0_OCW2, 0x67);  // ack to IRQ-07
}
