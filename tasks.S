#define FIRST_ARG 8
#define SECOND_ARG 12
//------------------------------------------------------------------------------
// TASK EXEC
//------------------------------------------------------------------------------
.text
.globl task_exec
task_exec:
        push %ebp
        movl %esp, %ebp
        movl SECOND_ARG(%ebp), %esp
        call *FIRST_ARG(%ebp)
        movl %ebp, %esp
        leave
        ret
//------------------------------------------------------------------------------
// TASK SWAP
//------------------------------------------------------------------------------
.text
.globl task_swap
task_swap:
        // void task_swap(uintptr_t *esp);
        // Pone en ejecución la tarea cuyo stack está en ‘*esp’, cuyo
        // valor se intercambia por el valor actual de %esp. Guarda y
        // restaura todos los callee-called registers.
        //  Registros que son callee-saved:
        // EBX - EBP - ESP - ESI
        // 28(%esp): Valor del stack antes de llamar a la funcion
        // 24(%esp): Argumento de la funcion &esp
        // 20(%esp): direccion de retorno
        push %esp              // 16(%esp)
        push %ebp              // 12(%esp)
        push %esi              // 8(%esp)
        push %ebx              // 4(%esp)

        leal 28(%esp), %ecx    // %ecx = %esp
        mov  %edi, 24(%esp)    // edi = *esp
        mov  24(%esp), %eax    // eax = *esp
        movl %eax, %esp        // %esp = *esp
        movl %ecx, %eax        // *esp = %esp
        /*
        ejemplo para saber como hacer para desreferenciar puntero y
        asignarle otra cosa.
        void f(int* c) {
            *c = 1;
        }
        codigo quivalente en assembly:
        push   %ebp
        mov    %esp, %ebp
        mov    %edi,-0x8(%ebp)
        mov    -0x8(%ebp), %eax
        movl   $0x1, (%eax)
        nop
        pop    %ebp
        ret
        */
        popl %ebx
        popl %esi
        popl %ebp
        popl %esp
        ret
//------------------------------------------------------------------------------
