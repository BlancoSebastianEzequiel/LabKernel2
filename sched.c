#include "lib/stdbool.h"
#include "lib/stddef.h"
#include "decls.h"
#include "sched.h"

#define MAX_TASK 10
// IF Bandera de interrupcion habilitada (bit 9: 0010 0000 0000)
#define IF 0x200

static struct Task Tasks[MAX_TASK];
static struct Task *current;
//------------------------------------------------------------------------------
// SCHED INIT
//------------------------------------------------------------------------------
void sched_init() {
    current = &Tasks[0];
    current->status = RUNNING;
    for (int i = 1; i < MAX_TASK; i++) {
        Tasks[i].status = FREE;
    }
}
//------------------------------------------------------------------------------
// TASK KILL
//------------------------------------------------------------------------------
static void task_kill() {
    current->status = DYING;
    sched(NULL);
}
//------------------------------------------------------------------------------
// SPAWN
//------------------------------------------------------------------------------
void spawn(void (*entry)(void)) {
    int i = 0;
    while (Tasks[i].status != FREE) i++;
    if (i == MAX_TASK) return;
    Tasks[i].status = READY;
    uint8_t* stack = &Tasks[i].stack[USTACK_SIZE];
    // cada uint32_t son cuatro casilleros de uint8_t y ademas son 11 porque
    //  uint16_t cs y uint16_t padding cuentan como uno de uint32_t
    size_t size = sizeof(struct TaskFrame);  // size deberia ser 44 = 11*4
    stack -= size;
    Tasks[i].frame = (struct TaskFrame *) (stack);
    Tasks[i].frame->edi = 0;
    Tasks[i].frame->esi = 0;
    Tasks[i].frame->ebp = 0;
    Tasks[i].frame->esp = 0;
    Tasks[i].frame->ebx = 0;
    Tasks[i].frame->edx = 0;
    Tasks[i].frame->ecx = 0;
    Tasks[i].frame->eax = 0;
    Tasks[i].frame->padding = 0;
    Tasks[i].frame->eflags = Tasks[i].frame->eflags | IF;
    Tasks[i].frame->eip = (uint32_t) entry;
    //  frame->cs tiene que tener el mismo valor que “segment” en idt_install().
    // Multiboot siempre define "8" como el segmento de código.
    // (Ver campo CS en `info registers` de QEMU.)
    static const uint8_t KSEG_CODE = 8;
    Tasks[i].frame->cs = KSEG_CODE;
    Tasks[i].frame->kill_fn = (uint32_t) task_kill;
}
//------------------------------------------------------------------------------
// SCHED
//------------------------------------------------------------------------------
void sched(struct TaskFrame *tf) {
    struct Task *new = 0;
    struct Task *old = current;
    /*
    Encontrar, de manera round-robin, la siguiente tarea que se encuentra en
    estado READY. Una posible manera es encontrar en el arreglo la tarea en
    ejecución, y buscar a partir de ahí:
    */
    enum TaskStatus oldStatus;
    bool foundRunning = false;
    bool foundReady = false;
    int seenCurrent = 0;
    int i = 0;
    while (seenCurrent < 2) {
        if (Tasks[i].status == RUNNING) {
            seenCurrent++;
            if (!foundRunning) {
                foundRunning = true;
                old = &Tasks[i];
                oldStatus = READY;
            }
        }
        if (Tasks[i].status == DYING) {
            seenCurrent++;
            if (!foundRunning) {
                foundRunning = true;
                old = &Tasks[i];
                oldStatus = FREE;
            }
        }
        if (Tasks[i].status == READY && foundRunning) {
            foundReady = true;
            new = &Tasks[i];
        }
        i++;
        if (i == MAX_TASK) i = 0;
    }
    /*
    Si se la encuentra, se debe poner old->status a READY y guardar en
    old->frame el frame recibido como parámetro; actualizar la variable global
    current y en current->status poner RUNNING. Por último, para ejecutar el
    cambio, se puede usar el siguiente assembler:
    */
    if (!foundReady) return;
    old->status = oldStatus;
    old->frame = tf;
    current = new;
    current->status = RUNNING;
    asm("movl %0, %%esp\n"
        "popa\n"
        "iret\n"
    :
    : "g"(current->frame)
    : "memory");
}
//------------------------------------------------------------------------------