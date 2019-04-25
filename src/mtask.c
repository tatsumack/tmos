#include <stdio.h>
#include "bootpack.h"

Timer* mt_timer;
int mt_tr;

TaskManager* taskman;
Timer* task_timer;

Task* task_init(MemoryManager* memman) {
    SegmentDescriptor* gdt = (SegmentDescriptor*)ADR_GDT;

    taskman = (TaskManager*)memman_alloc_4k(memman, sizeof(TaskManager));
    for (int i = 0; i < MAX_TASKS; i++) {
        taskman->tasks[i].status = taskstatus_notused;
        taskman->tasks[i].segment = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskman->tasks[i].tss, AR_TSS32);
    }

    for (int i = 0; i < MAX_TASKLEVELS; i++) {
        taskman->level[i].running = 0;
        taskman->level[i].now = 0;
    }
    taskman->now_lv = 0;

    Task* task = task_alloc();
    task->status = taskstatus_running;
    task->level = 0;
    task->priority = 2;
    task_add(task);
    task_switch_level();

    load_tr(task->segment);

    task_timer = timer_alloc();
    timer_settime(task_timer, task->priority);

    // idle
    Task* idle = task_alloc();
    idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
    idle->tss.eip = (int)&task_idle;
    idle->tss.es = 1 * 8;
    idle->tss.cs = 2 * 8;
    idle->tss.ss = 1 * 8;
    idle->tss.ds = 1 * 8;
    idle->tss.fs = 1 * 8;
    idle->tss.gs = 1 * 8;
    task_run(idle, MAX_TASKLEVELS - 1, 1);

    return task;
}

Task* task_alloc(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskman->tasks[i].status != taskstatus_notused) {
            continue;
        }

        Task* task = &taskman->tasks[i];
        task->status = taskstatus_alloc;
        task->tss.eflags = 0x00000202;  // IF = 1;
        task->tss.eax = 0;
        task->tss.ecx = 0;
        task->tss.edx = 0;
        task->tss.ebx = 0;
        task->tss.ebp = 0;
        task->tss.esi = 0;
        task->tss.edi = 0;
        task->tss.es = 0;
        task->tss.ds = 0;
        task->tss.fs = 0;
        task->tss.gs = 0;
        task->tss.ldtr = 0;
        task->tss.iomap = 0x40000000;
        return task;
    }
    TMOS_ERROR("can't find not used task");
    return 0;
}

void task_sleep(Task* task) {
    if (task->status != taskstatus_running) return;

    Task* now_task = task_now();
    task_remove(task);
    if (task == now_task) {
        task_switch_level();
        now_task = task_now();
        far_jmp(0, now_task->segment);
    }
}

void task_run(Task* task, int level, int priority) {
    if (level < 0) {
        level = task->level;
    }

    if (priority > 0) {
        task->priority = priority;
    }

    if (task->status == taskstatus_running && task->level != level) {
        task_remove(task);
    }

    taskman->is_lv_change = 1;

    if (task->status == taskstatus_running) return;

    task->level = level;
    task_add(task);
}

void task_switch(void) {
    TaskLevel* tl = &taskman->level[taskman->now_lv];
    Task* now_task = tl->tasks[tl->now];

    tl->now++;
    if (tl->now >= tl->running) tl->now = 0;

    if (taskman->is_lv_change) {
        task_switch_level();
        tl = &taskman->level[taskman->now_lv];
    }

    Task* new_task = tl->tasks[tl->now];
    timer_settime(task_timer, new_task->priority);
    if (new_task != now_task) {
        far_jmp(0, new_task->segment);
    }
}

Task* task_now(void) {
    TaskLevel* tl = &taskman->level[taskman->now_lv];
    return tl->tasks[tl->now];
}

void task_add(Task* task) {
    TaskLevel* tl = &taskman->level[task->level];
    tl->tasks[tl->running] = task;
    tl->running++;
    task->status = taskstatus_running;
}

void task_remove(Task* task) {
    TaskLevel* tl = &taskman->level[task->level];

    int i;
    for (i = 0; i < tl->running; i++) {
        if (tl->tasks[i] == task) {
            break;
        }
    }

    tl->running--;
    if (i < tl->now) {
        tl->now--;
    }

    if (tl->now >= tl->running) {
        tl->now = 0;
    }

    task->status = taskstatus_notused;

    for (; i < tl->running; i++) {
        tl->tasks[i] = tl->tasks[i + 1];
    }
}

void task_switch_level(void) {
    for (int i = 0; i < MAX_TASKLEVELS; i++) {
        if (taskman->level[i].running > 0) {
            taskman->now_lv = i;
            break;
        }
    }

    taskman->is_lv_change = 0;
}

void task_idle(void) {
    for (;;) {
        io_hlt();
    }
}
