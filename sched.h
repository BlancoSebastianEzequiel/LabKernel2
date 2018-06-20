#ifndef SCHED_H
#define SCHED_H

#include "lib/stdint.h"

enum TaskStatus {
    FREE = 0,
    READY = 1,
    RUNNING = 2,
    DYING = 3,
};

struct TaskFrame {
    // Ver ejercicio kern2-spawn más abajo.
};

struct Task {
    uint8_t stack[4096];
    enum TaskStatus status;
    struct TaskFrame *frame;
};

void sched_init();
void spawn(void (*entry)(void));
void sched(struct TaskFrame *tf);


#endif
