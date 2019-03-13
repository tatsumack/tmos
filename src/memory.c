#include "bootpack.h"

int is_486(void);

uint memtest(uint start, uint end) {
    int is486 = is_486();

    if (is486) {
        uint cr0 = (uint) load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    int result = asm_memtest(start, end);

    if (is486) {
        uint cr0 = (uint) load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return (uint) result;
}

int is_486() {
    uint eflags = (uint) io_load_eflags();
    eflags |= EFLAGS_AC_BIT;
    io_store_eflags(eflags);

    eflags = (uint) io_load_eflags();

    int result = 0;
    if ((eflags & EFLAGS_AC_BIT) != 0) {
        result = 1;
    }

    eflags &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflags);

    return result;
}
