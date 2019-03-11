#include "bootpack.h"

void init_pic(void) {
    // disable all interrupts
    io_out8(PIC0_IMR,  0xff  );
    io_out8(PIC1_IMR,  0xff  );

    // settings for PIC0
    io_out8(PIC0_ICW1, 0x11  );
    io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 map INT20-27 */
    io_out8(PIC0_ICW3, 1 << 2); // connect PIC1 through IRQ2
    io_out8(PIC0_ICW4, 0x01  );

    // settings for PIC1
    io_out8(PIC1_ICW1, 0x11  );
    io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 map INT28-2f */
    io_out8(PIC1_ICW3, 2     ); // connect PIC1 through IRQ2
    io_out8(PIC1_ICW4, 0x01  );

    // enable interrupts
    io_out8(PIC0_IMR,  0xfb  ); // 11111011 enable PIC1 only
    io_out8(PIC1_IMR,  0xff  ); // 11111111 disable all interrupts
}

#define PORT_KEYDAT 0x0060

FIFO keyfifo;

// interrupted by keyboard
void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61); // ack

    unsigned char data = (unsigned char)io_in8(PORT_KEYDAT);
    fifo_put(&keyfifo, data);
}

// interrupted by mouse
void inthandler2c(int *esp) {
    BootInfo* binfo = (BootInfo*) ADR_BOOTINFO;
    draw_rec(binfo->vram, binfo->width, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putstring8(binfo->vram, binfo->width, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
    for (;;) {
        io_hlt();
    }
}

// interrupted by PIC1 on initialization
void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67); // ack to IRQ-07
}
