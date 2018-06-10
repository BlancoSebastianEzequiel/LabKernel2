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
        push %ebx              // 16(%esp)
        push %ebp              // 12(%esp)
        push %esi              // 8(%esp)
        push %esp              // 4(%esp)
        leal 28(%esp), %ecx   // %ecx = %esp tarea activa
        movl 24(%esp), %eax   // eax = *esp
        movl %eax, %ecx   // %esp = *esp
        movl (%ecx), %eax     // esp = &%esp
        popl %esp
        popl %esi
        popl %ebp
        popl %ebx
        ret
//------------------------------------------------------------------------------
