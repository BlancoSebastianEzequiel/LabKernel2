#include "decls.h"
#include "sched.h"

#define MAX_TASK 10

static struct Task Tasks[MAX_TASK];
static struct Task *current;

void sched_init() {
    *current = Tasks[0];
    current->status = RUNNING;
}

void spawn(void (*entry)(void)) {
    // ...
}

void sched(struct TaskFrame *tf) {
    // ...
}