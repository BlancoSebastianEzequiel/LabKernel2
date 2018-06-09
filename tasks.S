#define FIRST_ARG 8
#define SECOND_ARG 12
.text
.globl task_exec
task_exec:
        //
        push %ebp
        movl %esp, %ebp
        movl SECOND_ARG(%ebp), %esp
        call FIRST_ARG(%ebp)
        movl %ebp, %esp
        leave
        ret
