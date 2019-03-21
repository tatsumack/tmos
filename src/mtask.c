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

    Task* task = task_alloc();
    task->status = taskstatus_running;
    taskman->running = 1;
    taskman->now = 0;
    taskman->orders[0] = task;
    load_tr(task->segment);

    task_timer = timer_alloc();
    timer_settime(task_timer, 2);

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

    char needTaskSwitch = 0;
    if (task == taskman->orders[taskman->now]) {
        needTaskSwitch = 1;
    }

    int i;
    for (i = 0; i < taskman->running; i++) {
        if (taskman->orders[i] == task) {
            // found
            break;
        }
    }

    taskman->running--;
    if (i < taskman->now) {
        taskman->now--;
    }

    for (; i < taskman->running; i++) {
        taskman->orders[i] = taskman->orders[i + 1];
    }

    task->status = taskstatus_notused;

    if (needTaskSwitch) {
        if (taskman->now >= taskman->running) {
            taskman->now = 0;
        }
        far_jmp(0, taskman->orders[taskman->now]->segment);
    }
}

void task_run(Task* task) {
    task->status = taskstatus_running;
    taskman->orders[taskman->running] = task;
    taskman->running++;
}

void task_switch(void) {
    timer_settime(task_timer, 2);

    if (taskman->running == 1) return;

    taskman->now++;
    taskman->now %= taskman->running;

    far_jmp(0, taskman->orders[taskman->now]->segment);
}
