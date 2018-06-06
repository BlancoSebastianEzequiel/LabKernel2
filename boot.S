#include "multiboot.h"

#define KSTACK_SIZE 8192
#define LOCAL_SIZE 4

.align 4
multiboot:
    .long MULTIBOOT_HEADER_MAGIC
    .long 0
    .long -(MULTIBOOT_HEADER_MAGIC)

.globl _start
_start:
    // Paso 1: Configurar el stack antes de llamar a kmain.
    movl $0, %ebp
    movl kstack, %esp
    push %ebp
    mov  %ebp, %esp
    # space allocated on the stack for the local variables
    sub %esp, LOCAL_SIZE # space allocated on the stack for the local variables

    // Paso 2: pasar la información multiboot a kmain. Si el
    // kernel no arrancó vía Multiboot, se debe pasar NULL.
    //
    // Usar una instrucción de comparación (TEST o CMP) para
    // comparar con MULTIBOOT_BOOTLOADER_MAGIC, pero no usar
    // un salto a continuación, sino una instrucción CMOVcc
    // (copia condicional).
    // ...
    cmp %eax, MULTIBOOT_BOOTLOADER_MAGIC # si arranco con multiboot -> ZF = 1
    jz null # si no arranco con multiboot -> paso un null
    push %eax
    jmp run_kmain

null:
    push 0
run_kmain:
    call kmain
halt:
    hlt
    jmp halt

.data
.p2align 12
kstack:
    .space KSTACK_SIZE