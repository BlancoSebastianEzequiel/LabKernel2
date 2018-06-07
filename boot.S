#include "multiboot.h"

#define KSTACK_SIZE 8192
#define FLAGS 0
#define CRC ( -(MULTIBOOT_HEADER_MAGIC + FLAGS) )

.align 4
multiboot:
    .long MULTIBOOT_HEADER_MAGIC
    .long FLAGS
    .long CRC

.globl _start
_start:
    // Paso 1: Configurar el stack antes de llamar a kmain.
    movl $0, %ebp
    movl $kstack, %esp
    push %ebp
    movl %esp, %ebp

    // Paso 2: pasar la información multiboot a kmain. Si el
    // kernel no arrancó vía Multiboot, se debe pasar NULL.
    //
    // Usar una instrucción de comparación (TEST o CMP) para
    // comparar con MULTIBOOT_BOOTLOADER_MAGIC, pero no usar
    // un salto a continuación, sino una instrucción CMOVcc
    // (copia condicional).
    mov $0, %ecx
    // si arranco con multiboot -> ZF = 1
    cmp %eax, MULTIBOOT_BOOTLOADER_MAGIC
    cmovne %ecx, %ebx
    push %ebx
    call kmain
halt:
    hlt
    jmp halt

.data
.p2align 12
kstack:
    .space KSTACK_SIZE