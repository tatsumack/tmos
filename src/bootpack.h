#ifndef TMOC_BOOTPACK_H
#define TMOC_BOOTPACK_H

typedef unsigned int uint;
typedef unsigned char uchar;

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

#define ADR_BOOTINFO    0x00000ff0
#define ADR_MEMMAN      0x003c0000

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

// util.c
int min(int a, int b);

int max(int a, int b);

int clamp(int x, int a, int b);

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
    uchar* buf;
    int write, read, size, free, flags;
} FIFO;

void fifo_init(FIFO* fifo, int size, uchar* buf);

int fifo_put(FIFO* fifo, uchar data);

int fifo_get(FIFO* fifo);

int fifo_empty(FIFO* fifo);

// graphic.c
void init_palette(void);

void init_screen(char* vram, int width, int height);

void init_mouse_cursor8(char* mouse, char bc);

void set_palette(uchar* rgb, int size);

void draw_rec(char* vram, int width, uchar c, int x0, int y0, int x1, int y1);

void putfont8(char* vram, int width, int x, int y, char color, char* font);

void putstring8(char* vram, int width, int x, int y, char color, char* s);

void putblock8_8(char* vram, int width, int pwidth, int pheight, int px0, int py0, char* buf, int bwidth);

// desctbl.c
void init_gdtidt(void);

void set_segmdesc(SegmentDescriptor* sd, uint limit, int base, int ar);

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

#define KEY_FIFOBUF             32
#define PORT_KEYDAT             0x0060
#define PORT_KEYSTA             0x0064
#define PORT_KEYCMD             0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE       0x60
#define KBC_MODE                0x47

// mouse.c
#define MOUSE_FIFOBUF           128
#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4

typedef struct MouseInfo {
    int x, y;
    char image[256];
} MouseInfo;

typedef struct MouseDec {
    uchar buf[3], phase;
    int x, y, btn;
} MouseDec;

void init_mouse(MouseInfo* minfo, MouseDec* mdec);

int mouse_decode(MouseDec* mdec, uchar dat);

void mouse_move(MouseInfo* minfo, int dx, int dy);

// memory.c
#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000
#define MEMMAN_FREES        4090

typedef struct FreeInfo {
    uint addr, size;
} FreeInfo;

typedef struct MemoryManager {
    int free_num, max_free_num, fail_size, fail_num;
    FreeInfo free[MEMMAN_FREES];
} MemoryManager;

void init_memory();

uint memtest(uint start, uint end);

void memman_init(MemoryManager* man);

uint memman_total_free_size(MemoryManager* man);

uint memman_alloc(MemoryManager* man, uint size);

int memman_free(MemoryManager* man, uint addr, uint size);

uint memman_alloc_4k(MemoryManager* man, uint size);

int memman_free_4k(MemoryManager* man, uint addr, uint size);

// sheet.c
#define MAX_SHEETS 256

typedef struct Sheet {
    char* buf;
    int width, height, vx, vy, col_inv, zorder, flags;
    struct SheetManager* man;
} Sheet;

typedef struct SheetManager {
    char* vram;
    int width, height, top;
    Sheet sheets[MAX_SHEETS];
    Sheet* zorders[MAX_SHEETS];
} SheetManager;

SheetManager* shtman_init(MemoryManager* memman, char* vram, int width, int height);

Sheet* sheet_alloc(SheetManager* shtman);

void sheet_free(Sheet* sht);

void sheet_set_buf(Sheet* sht, uchar* buf, int width, int height, int col_inv);

void sheet_updown(Sheet* sht, int zorder);

void sheet_refresh(Sheet* sht, int bx0, int by0, int bx1, int by1);

void sheet_refresh_sub(SheetManager* shtman, int vx0, int vy0, int vx1, int vy1);

void sheet_slide(Sheet* sht, int vx0, int vy0);

// window.c
void make_window(uchar *buf, int width, int height, char *title);

// debug.c
void debug_error(char* s, char* file, int line);

#define TMOC_ERROR(...) do { char error_buf[100]; sprintf(error_buf, __VA_ARGS__); debug_error(error_buf, __FILE__, __LINE__); } while(0)

#endif // TMOC_BOOTPACK_H
