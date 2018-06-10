.globl vga_write2
vga_write2:
        push %ebp
        movl %esp, %ebp
        /*
        On x86-32 targets, the regparm attribute causes the compiler to pass
        arguments number one to number if they are of integral type in registers
        EAX, EDX, and ECX instead of on the stack. Functions that take a
        variable number of arguments continue to be passed all of their
        arguments on the stack.
        Beware that on some ELF systems this attribute is unsuitable for global
        functions in shared libraries with lazy binding (which is the default).
        Lazy binding sends the first call via resolving code in the loader,
        which might assume EAX, EDX and ECX can be clobbered, as per the
        standard calling conventions. Solaris 8 is affected by this. Systems
        with the GNU C Library version 2.1 or higher and FreeBSD are believed to
        be safe since the loaders there save EAX, EDX and ECX. (Lazy binding can
        be disabled with the linker or the loader if desired, to avoid the
        problem.)
        */
        push %ecx         // 3rd argument (color)
        push %edx         // 2nd argument (linea)
        push %eax         // 1st argument (msg)
        call vga_write
        // movl %esp, %ebp
        leave
        ret