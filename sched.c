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
    for (int i = 0; i < MAX_TASK; ++i) {
        if (Tasks[i].status != FREE) continue;
        Tasks[i].status = READY;
        Tasks[i].frame->edi = 0;
        Tasks[i].frame->esi = 0;
        Tasks[i].frame->ebp = 0;
        Tasks[i].frame->esp = 0;
        Tasks[i].frame->ebx = 0;
        Tasks[i].frame->edx = 0;
        Tasks[i].frame->ecx = 0;
        Tasks[i].frame->eax = 0;
        Tasks[i].frame->padding = 0;
        /*
         * %eflags debe contener el flag IF, o las interrupciones de reloj no
         * se habilitarán al entrar la tarea en ejecución.
        */
        if (Tasks[i].frame->eflags != IF) {
            Tasks[i].frame->eflags = IF;
        }
        break;
    }
}

void sched(struct TaskFrame *tf) {
    // ...
}