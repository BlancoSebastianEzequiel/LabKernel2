//------------------------------------------------------------------------------
//        COMENTARIOS Y OPCIONES DE LOS DISTINTOS BREAKPOINT
//MANEJADOR MINIMO
/*.globl breakpoint
breakpoint:
     //Manejador minimo.
     iret
*/
//FIN MANEJADOR MINIMO

//SESION GDB

//VERSION A
/*.globl breakpoint
breakpoint:
        nop
        test %eax, %eax
        iret
*/
//FIN VERSION A

//VERSION B
/*.globl breakpoint
breakpoint:
        nop
        test %eax, %eax
        ret
*/
//FIN VERSION B
//FIN SESION GDB

/*VERSION FINAL DEL BREAKPOINT

.globl breakpoint
breakpoint:
    // (1) Guardar registros.
        //OPCION A
                //pusha //pushea todos los registros AX CX DX BX SP BP SI DI
                //movl %esp, %ebp
        //OPCION B
                //Debo pushear ebp y guardar el valor de esp en ebp
                //push %ebp
                //movl %esp, %ebp
                //push %eax
                //push %edx
                //push %ecx
        //OPCION C
                //Debo pushear ebp y guardar el valor de esp en ebp
                //push %ebp
                //movl %esp, %ebp
                //push %ebx
                //push %esi
                //push %edi
    // (2) Preparar argumentos de la llamada.
        //Para vga_write2 los argumentos deben estar en eax, edx y ecx
        //movl $0xB0, %ecx
        //movl $14, %edx
        //movl $breakpoint_msg, %eax
    // (3) Invocar a vga_write2()
        //call vga_write2
    // (4) Restaurar registros.
        //OPCION A
                //popa
        //OPCION B
                //pop %ecx
                //pop %edx
                //pop %eax
                //Debo desalocar variables locales y recuperar el ebp puseado
                //movl %ebp, %esp
                //pop %ebp
        //OPCION C
                pop %edi
                pop %esi
                pop %ebx
                //Debo desalocar variables locales y recuperar el ebp puseado
                //movl %ebp, %esp
                //pop %ebp
    // (5) Finalizar ejecución del manejador.
        //iret

.data
breakpoint_msg:
        .asciz "Hello, breakpoint"

FIN VERSION FINAL BREAKPOINT*/
//------------------------------------------------------------------------------


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
        call keyboard
        iret
//------------------------------------------------------------------------------
// TIMER ASM
//------------------------------------------------------------------------------
.globl timer_asm
timer_asm:
        /*
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
        */
        // Guardar registros e invocar handler
        pusha
        call timer

        // Ack *antes* de llamar a sched()
        movl $ACK_IRQ, %eax
        outb %al, $PIC1

        // Llamada a sched con argumento
        push %esp
        call sched

        // Retornar (si se volvió de sched)
        addl $4, %esp
        popa
        iret
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
