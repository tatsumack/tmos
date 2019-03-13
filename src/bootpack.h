#define COL8_000000        0
#define COL8_FF0000        1
#define COL8_00FF00        2
#define COL8_FFFF00        3
#define COL8_0000FF        4
#define COL8_FF00FF        5
#define COL8_00FFFF        6
#define COL8_FFFFFF        7
#define COL8_C6C6C6        8
#define COL8_840000        9
#define COL8_008400        10
#define COL8_848400        11
#define COL8_000084        12
#define COL8_840084        13
#define COL8_008484        14
#define COL8_848484        15


#define ADR_BOOTINFO 0x00000ff0
typedef struct BootInfo {
    char cyles, leds, vmode, reserve;
    short width, height;
    char* vram;
} BootInfo;

typedef struct SegmentDescriptor {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
} SegmentDescriptor;

typedef struct GateDescriptor {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
} GateDescriptor;


// asmfunc.asm
void io_hlt(void);

void io_cli(void);

void io_sti(void);

void io_stihlt(void);

void io_out8(int port, int data);

int io_in8(int port);

int io_load_eflags(void);

void io_store_eflags(int eflags);

int load_cr0(void);

void store_cr0(int cr0);

void load_gdtr(int limit, int addr);

void load_idtr(int limit, int addr);

void asm_inthandler21(void);

void asm_inthandler27(void);

void asm_inthandler2c(void);

int asm_memtest(int start, int end);

// fifo.c
typedef struct FIFO {
    unsigned char* buf;
    int write, read, size, free, flags;
} FIFO;

void fifo_init(FIFO* fifo, int size, unsigned char* buf);

int fifo_put(FIFO* fifo, unsigned char data);

int fifo_get(FIFO* fifo);

int fifo_empty(FIFO* fifo);

// graphic.c
void init_palette(void);

void init_screen(char* vram, int width, int height);

void init_mouse_cursor8(char* mouse, char bc);

void set_palette(unsigned char* rgb, int size);

void draw_rec(char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1);

void putfont8(char* vram, int width, int x, int y, char color, char* font);

void putstring8(char* vram, int width, int x, int y, char color, unsigned char* s);

void putblock8_8(char* vram, int width, int pwidth, int pheight, int px0, int py0, char* buf, int bwidth);

// desctbl.c
void init_gdtidt(void);

void set_segmdesc(SegmentDescriptor* sd, unsigned int limit, int base, int ar);

void set_gatedesc(GateDescriptor* gd, int offset, int selector, int ar);

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

// int.c
void init_pic(void);

void inthandler21(int* esp);

void inthandler27(int* esp);

void inthandler2c(int* esp);

#define PIC0_ICW1        0x0020
#define PIC0_OCW2        0x0020
#define PIC0_IMR        0x0021
#define PIC0_ICW2        0x0021
#define PIC0_ICW3        0x0021
#define PIC0_ICW4        0x0021
#define PIC1_ICW1        0x00a0
#define PIC1_OCW2        0x00a0
#define PIC1_IMR        0x00a1
#define PIC1_ICW2        0x00a1
#define PIC1_ICW3        0x00a1
#define PIC1_ICW4        0x00a1

// keyboard.c
void init_keyboard(void);

void wait_kbc_sendready(void);

#define PORT_KEYDAT                0x0060
#define PORT_KEYSTA                0x0064
#define PORT_KEYCMD                0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE        0x60
#define KBC_MODE                0x47

// mouse.c
typedef struct MouseDec {
    unsigned char buf[3], phase;
    int x, y, btn;
} MouseDec;

void enable_mouse(MouseDec* mdec);

int mouse_decode(MouseDec* mdec, unsigned char dat);


#define KEYCMD_SENDTO_MOUSE        0xd4
#define MOUSECMD_ENABLE            0xf4

// memory.c
#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

unsigned int memtest(unsigned int start, unsigned int end);
