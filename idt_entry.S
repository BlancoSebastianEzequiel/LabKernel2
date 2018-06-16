//------------------------------------------------------------------------------
// BREAKPOINT
//------------------------------------------------------------------------------
.globl breakpoint
breakpoint:
        // (1) Guardar registros.
        push %eax
        push %edx
        push %ecx
        // (2) Preparar argumentos de la llamada.
        // vga_write2("Hello, breakpoint", 14, 0xB0);
        movl $0xB0, %ecx                   // 3rd argument (color)
        movl $14, %edx                     // 2nd argument (linea)
        movl $breakpoint_msg, %eax         // 1st argument (msg)
        // (3) Invocar a vga_write2()
        call vga_write2
        // (4) Restaurar registros.
        pop %ecx
        pop %edx
        pop %eax
        // (5) Finalizar ejecución del manejador.
        iret

.data
breakpoint_msg:
        .asciz "Hello, breakpoint"
//------------------------------------------------------------------------------
// ACK IRQ
//------------------------------------------------------------------------------
#define PIC1 0x20
#define ACK_IRQ 0x20
.globl ack_irq
ack_irq:
        // Indicar que se manejó la interrupción.
        movl $ACK_IRQ, %eax
        outb %al, $PIC1
        iret
//------------------------------------------------------------------------------
// TIMER ASM
//------------------------------------------------------------------------------
.globl timer_asm
timer_asm:
        // Guardar registros.
        push %eax
        push %edx
        push %ecx
        call timer
        // Restaurar registros.
        pop %ecx
        pop %edx
        pop %eax
        jmp ack_irq
//------------------------------------------------------------------------------
// DIVZERO
//------------------------------------------------------------------------------
.globl divzero
divzero:
        // El manejador debe, primero, incrementar el valor de %ebx, de manera
        // que cuando se reintente la instrucción, ésta tenga éxito.
        mov $1, %ebx
        // (1) Guardar registros.
        push %eax
        push %edx
        push %ecx
        // vga_write_cyan("Se divide por ++ebx", 17);
        movl $17, %edx    // 2nd argument (linea)
        movl $msg, %eax   // 1st argument (msg)
        // (3) Invocar a vga_write2()
        call vga_write_cyan
        // (4) Restaurar registros.
        pop %ecx
        pop %edx
        pop %eax
        iret
.data
msg:
        .asciz "Se divide por ++ebx"
//------------------------------------------------------------------------------
