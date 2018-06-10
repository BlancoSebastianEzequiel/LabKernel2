#include "decls.h"

#define COUNTLEN 20
#define TICKS (1ULL << 15)
#define DELAY(x) (TICKS << (x))
#define USTACK_SIZE 4096

static volatile char *const VGABUF = (volatile void *) 0xb8000;
//char msg[256] = "";
static uintptr_t esp;
static uint8_t stack1[USTACK_SIZE] __attribute__((aligned(4096)));
static uint8_t stack2[USTACK_SIZE] __attribute__((aligned(4096)));
//------------------------------------------------------------------------------
// YIELD
//------------------------------------------------------------------------------
static void yield() {
    if (esp) {
        //fmt_int(esp, msg, 256);
        task_swap(&esp);
        //vga_write(msg, 22, 0x2F);
    }
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
// CONTADOR RUN
//------------------------------------------------------------------------------
void contador_run() {
    // Configurar stack1 y stack2 con los valores apropiados.
    uintptr_t *a = (uintptr_t*) stack1+USTACK_SIZE;
    uintptr_t *b = (uintptr_t*) stack2+USTACK_SIZE;
    *(--a) = 0x2F;  // color
    *(--a) = 0;  // linea
    *(--a) = 100;  // numero

    /*
     la configuración del segundo contador es más compleja, y seguramente sea
     mejor realizarla tras implementar task_swap(); pues se debe crear
     artificialmente el stack tal y como lo hubiera preparado esta función.
    */
    *(--b) = 0x4F;  // color
    *(--b) = 1;  // linea
    *(--b) = 100;  // numero

    // Actualizar la variable estática ‘esp’ para que apunte
    // al del segundo contador.
    esp = (uintptr_t) b;

    // push %esp
    b--;
    asm("movl %%esp, %0;": /* no outputs */: "r"(b));
    // push %ebp
    b--;
    asm("movl %%ebp, %0;": /* no outputs */: "r"(b));
    // push %esi
    b--;
    asm("movl %%esi, %0;": /* no outputs */: "r"(b));
    // push %ebx
    b--;
    asm("movl %%ebx, %0;": /* no outputs */: "r"(b));

    // Lanzar el primer contador con task_exec.
    task_exec((uintptr_t) contador_yield, (uintptr_t) a);
    //contador_yield(100, 0, 0x2F);
    //contador_yield(100, 1, 0x4F);
}
//------------------------------------------------------------------------------
