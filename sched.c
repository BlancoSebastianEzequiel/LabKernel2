#include "decls.h"
#include "sched.h"

#define MAX_TASK 10
// IF Bandera de interrupción habilitada (bit 9: 0010 0000 0000)
#define IF 0x200

static struct Task Tasks[MAX_TASK];
static struct Task *current;
//------------------------------------------------------------------------------
// SCHED INIT
//------------------------------------------------------------------------------
void sched_init() {
    current = &Tasks[0];
    current->status = RUNNING;
}
//------------------------------------------------------------------------------
// SPAWN
//------------------------------------------------------------------------------
void spawn(void (*entry)(void)) {
    /*
    Una función spawn() que, dada una función o entry point, deja preparado un
    struct Task para que la tarea entre en ejecución.
    La función spawn() debe:

        *Encontrar en el arreglo global Tasks, una entrada con estado FREE

        *Cambiar su status a READY

        *Inicializar todos sus registros a cero, excepto %cs, %eip y %eflags.
        En particular %eflags debe contener el flag IF, o las interrupciones de
        reloj no se habilitarán al entrar la tarea en ejecución.
    */
    int i = 0;
    while (Tasks[i].status != FREE) i++;
    if (i == MAX_TASK) return;
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
    uint32_t eflag = Tasks[i].frame->eflags;
    if (!(eflag & IF)) Tasks[i].frame->eflags = eflag | IF;
    uint8_t* stack = &Tasks[i].stack[USTACK_SIZE];
    stack -= 10;
    stack[0] = (uint8_t) 0;
    stack[1] = (uint8_t) entry;
    stack[2] = (uint8_t) Tasks[i].frame->edi;
    stack[3] = (uint8_t) Tasks[i].frame->esi;
    stack[4] = (uint8_t) Tasks[i].frame->ebp;
    stack[5] = (uint8_t) Tasks[i].frame->esp;
    stack[6] = (uint8_t) Tasks[i].frame->ebx;
    stack[7] = (uint8_t) Tasks[i].frame->edx;
    stack[8] = (uint8_t) Tasks[i].frame->ecx;
    stack[9] = (uint8_t) Tasks[i].frame->eax;
}
//------------------------------------------------------------------------------
// PRINT
//------------------------------------------------------------------------------
void print(uint64_t value, int8_t line) {
    char buf[256];
    fmt_int(value, buf, 256);
    vga_write(buf, line, 0x2F);
    buf[0] = '\0';
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

    static int cuenta = 0;
    print((uint64_t) ++cuenta, 6);

    bool foundCurrent = false;
    bool foundReady = false;
    for (int i = 0; i < MAX_TASK; i++) {
        if (Tasks[i].status == RUNNING && !foundCurrent) {
            foundCurrent = true;
            old = &Tasks[i];
        } else if (Tasks[i].status == READY && foundCurrent) {
            foundReady = true;
            new = &Tasks[i];
            break;
        }
    }
    /*
    Si se la encuentra, se debe poner old->status a READY y guardar en
    old->frame el frame recibido como parámetro; actualizar la variable global
    current y en current->status poner RUNNING. Por último, para ejecutar el
    cambio, se puede usar el siguiente assembler:
    */
    if (!foundReady) return;
    old->status = READY;
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
