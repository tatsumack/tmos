#include "bootpack.h"

int is_486(void);

unsigned int memtest(unsigned int start, unsigned int end) {
    int is486 = is_486();

    if (is486) {
        unsigned int cr0 = (unsigned int) load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    int result = asm_memtest(start, end);

    if (is486) {
        unsigned int cr0 = (unsigned int) load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return (unsigned int) result;
}

int is_486() {
    unsigned int eflags = (unsigned int) io_load_eflags();
    eflags |= EFLAGS_AC_BIT;
    io_store_eflags(eflags);

    eflags = (unsigned int) io_load_eflags();

    int result = 0;
    if ((eflags & EFLAGS_AC_BIT) != 0) {
        result = 1;
    }

    eflags &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflags);

    return result;
}
