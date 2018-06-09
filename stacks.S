#define USTACK_SIZE 4096
#define STACK_DIR -12
#define FIRST_ARG -12
#define SECOND_ARG -8
#define THIRD_ARG -4


.data
        .align 4096
stack1:
        .space USTACK_SIZE
stack1_top:

        .p2align 12
stack2:
        .space USTACK_SIZE
stack2_top:

msg1:
        .asciz "vga_write() from stack1"
msg2:
        .asciz "vga_write() from stack2"

// esqueleto de implementación:
.text
.globl two_stacks
two_stacks:
        // Preámbulo estándar
        push %ebp
        movl %esp, %ebp

        // Registros para apuntar a stack1 y stack2.
        mov $stack1_top, %eax
        mov $stack2_top, %ebx   // Decidir qué registro usar.
        // (Cuando use %ecx no andaba!!! preguntar por que)

        // Cargar argumentos a ambos stacks en paralelo. Ayuda:
        // usar offsets respecto a %eax ($stack1_top), y lo mismo
        // para el registro usado para stack2_top.
        movl $0x17, THIRD_ARG(%eax)
        movl $0x90, THIRD_ARG(%ebx)

        movl $12, SECOND_ARG(%eax)
        movl $13, SECOND_ARG(%ebx)

        movl $msg1, FIRST_ARG(%eax)
        movl $msg2, FIRST_ARG(%ebx)

        // Realizar primera llamada con stack1. Ayuda: usar LEA
        // con el mismo offset que los últimos MOV para calcular
        // la dirección deseada de ESP.
        leal STACK_DIR(%eax), %esp
        call vga_write

        // Restaurar stack original. ¿Es %ebp suficiente?
        movl %ebp, %esp

        // Realizar segunda llamada con stack2.
        leal STACK_DIR(%ebx), %esp
        call vga_write

        // Restaurar registros callee-saved, si se usaron.

        leave
        ret


