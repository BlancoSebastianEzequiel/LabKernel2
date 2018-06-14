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
        // EBX - EBP - EDI - ESI
        // 24(%esp): Valor del stack antes de llamar a la funcion
        // 20(%esp): Argumento de la funcion uintptr_t *esp
        // 16(%esp): direccion de retorno
        push %edi              // 12(%esp)
        push %ebp              // 8(%esp)
        push %esi              // 4(%esp)
        push %ebx              // 0(%esp)

        mov  20(%esp), %edx
        mov  %esp, %eax
        mov  (%edx), %esp
        mov  %eax, (%edx)

        pop %ebx
        pop %esi
        pop %ebp
        pop %edi
        ret
//------------------------------------------------------------------------------
