#include "decls.h"
#include "lib/string.h"
#include "sched.h"

#define COUNTLEN 20
#define TICKS (1ULL << 15)
#define DELAY(x) (TICKS << (x))
#define USTACK_SIZE 4096

static volatile char *const VGABUF = (volatile void *) 0xb8000;

static uintptr_t esp;
static uint8_t stack1[USTACK_SIZE] __attribute__((aligned(4096)));
static uint8_t stack2[USTACK_SIZE] __attribute__((aligned(4096)));
//------------------------------------------------------------------------------
// EXIT
//------------------------------------------------------------------------------
static void exit() {
    uintptr_t tmp = esp;
    esp = 0;
    if (tmp)
        task_swap(&tmp);
}
//------------------------------------------------------------------------------
// YIELD
//------------------------------------------------------------------------------
static void yield() {
    if (esp)
        task_swap(&esp);
}
//------------------------------------------------------------------------------
// CONTADOR YIELD
//------------------------------------------------------------------------------
static void contador_yield(unsigned lim, uint8_t linea, char color) {
    char counter[COUNTLEN] = {'0'};  // ASCII digit counter (RTL).

    while (lim--) {
        char *c = &counter[COUNTLEN];
        volatile char *buf = VGABUF + 160 * linea + 2 * (80 - COUNTLEN);

        unsigned p = 0;
        unsigned long long i = 0;

        while (i++ < DELAY(6))  // Usar un entero menor si va demasiado lento.
            ;

        while (counter[p] == '9') {
            counter[p++] = '0';
        }

        if (!counter[p]++) {
            counter[p] = '1';
        }

        while (c-- > counter) {
            *buf++ = *c;
            *buf++ = color;
        }
        yield();
    }
}
//------------------------------------------------------------------------------
// CONTADOR 1
//------------------------------------------------------------------------------
static void contador1() {
    contador_yield(100, 2, 0x2F);
}
//------------------------------------------------------------------------------
// CONTADOR 2
//------------------------------------------------------------------------------
static void contador2() {
    contador_yield(100, 3, 0x6F);
}
//------------------------------------------------------------------------------
// CONTADOR 3
//------------------------------------------------------------------------------
static void contador3() {
    contador_yield(100, 4, 0x4F);
}
//------------------------------------------------------------------------------
// CONTADOR SPAWN
//------------------------------------------------------------------------------
void contador_spawn() {
    spawn(contador1);
    spawn(contador2);
    spawn(contador3);
}
//------------------------------------------------------------------------------
// CONTADOR RUN
//------------------------------------------------------------------------------
void contador_run() {
    // Configurar stack1 y stack2 con los valores apropiados.
    uintptr_t *a = (uintptr_t*) &stack1[USTACK_SIZE];
    uintptr_t *b = (uintptr_t*) &stack2[USTACK_SIZE];

    //contador_yield(100, 0, 0x2F);
    *(--a) = 0x2F;  // color
    *(--a) = 0;  // linea
    *(--a) = 100;  // numero

    /*
     la configuración del segundo contador es más compleja, y seguramente sea
     mejor realizarla tras implementar task_swap(); pues se debe crear
     artificialmente el stack tal y como lo hubiera preparado esta función.
    */
    //contador_yield(100, 1, 0x4F);
    *(--b) = 0x4F;  // color
    *(--b) = 1;     // linea
    *(--b) = 10;   // numero
    *(--b) = (uintptr_t) exit;                //ret falso
    *(--b) = (uintptr_t) contador_yield;      //ret cominezo
    *(--b) = 0;     //push %edi
    *(--b) = 0;     //push %ebp
    *(--b) = 0;     //push %esi
    *(--b) = 0;     //push %ebx
    // Actualizar la variable estática ‘esp’ para que apunte
    // al del segundo contador.
    esp = (uintptr_t) b;

    // Lanzar el primer contador con task_exec.
    task_exec((uintptr_t) contador_yield, (uintptr_t) a);
}
//------------------------------------------------------------------------------
