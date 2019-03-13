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

void memman_init(MemoryManager* man) {
    man->free_num = 0;
    man->max_free_num = 0;
    man->fail_num = 0;
    man->fail_size = 0;
}

uint memman_total_free_size(MemoryManager* man) {
    uint size = 0;
    for (int i = 0; i < man->free_num; i++) {
        size += man->free[i].size;
    }
    return size;
}

uint memman_alloc(MemoryManager* man, uint size) {
    for (int i = 0; i < man->free_num; i++) {
        FreeInfo free = man->free[i];
        if (free.size < size) continue;

        // found enough free space

        uint addr = free.addr;
        free.addr += size;
        free.size -= size;

        if (free.size == 0) {
            man->max_free_num--;
            for (; i < man->free_num; i++) {
                man->free[i] = man->free[i + 1];
            }
        }

        return addr;
    }

    return 0;
}

int memman_free(MemoryManager* man, uint addr, uint size) {
    int i;
    for (i = 0; i < man->free_num; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }

    // free[i-1] < addr < free[i]
    if (i > 0 && man->free[i - 1].addr + man->free[i - 1].size == addr) {
        man->free[i - 1].size += size;

        if (i < man->free_num) {
            if (addr + size == man->free[i].addr) {
                man->free[i - 1].size += man->free[i].size;
                man->free_num--;
                for (; i < man->free_num; i++) {
                    man->free[i] = man->free[i + 1];
                }
            }

        }
        return 0;
    }

    if (i < man->free_num && addr + size == man->free[i].addr) {
        man->free[i].addr = addr;
        man->free[i].size += size;
        return 0;
    }

    if (man->free_num < MEMMAN_FREES) {
        for (int j = man->free_num; j > i; j--) {
            man->free[j] = man->free[j - 1];
        }
        man->free_num++;

        if (man->max_free_num < man->free_num) {
            man->max_free_num = man->free_num;
        }

        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    man->fail_num++;
    man->fail_size += size;
    return -1;
}
