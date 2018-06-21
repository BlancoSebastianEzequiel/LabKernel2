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
    /*
    Te respondo por partes:

        El entry point se coloca, de manera similar a ejercicios anteriores,
        para que ret/iret funcione: “en el tope de la pila una vez hecho pop de
        los registros de propósito general.”

        En este caso es más fácil porque tenés el miembro “eip” del struct
        diciéndote dónde hay que almacenarlo.

        Fijate que el “taskframe” es siempre un puntero, y siempre apuntará a
        alguna región de la pila.

        Dicho de otra manera: en ejercicios anteriores, para todas las tareas
        tuvimos que reservar un stack; en este ejercicio, se reserva
        directamente como parte del struct (podría haber sido en cualquier otro
        sitio, pero esto es más fácil).

        Cuando sched() recibe un puntero a TaskFrame, podés imaginar también que
        recibiera un void* o un uintptr_t*: lo que recibe es el puntero al tope
        de la pila. Ocurre que, en esta configuración, el tope de la pila de una
        tarea que no está en ejecucións siempre “coincide” con un TaskFrame, de
        ahí su uso.

        Técnicamente sched() no necesita que sea un TaskFrame*.

        Espero que esto ayude a aclarar tus dudas, para nuevas consultas no
        dudes en escribirnos de nuevo.
    */
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
