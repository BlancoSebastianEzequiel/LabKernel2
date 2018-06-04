# Un kernel con reloj y tres tareas

Material de apoyo:

  - [decls.h](decls.h)
  - [interrupts.h](interrupts.h)
  - [multiboot.h](multiboot.h)

## Ãndice
{:.no_toc}

* TOC
{:toc}


## CreaciÃ³n de stacks en el kernel
{: #kernstack}

Cuando un programa se ejecuta, normalmente es el sistema operativo quien le configura el stack de manera adecuada, esto es: reserva un espacio en memoria (a menudo 4 u 8Â KiB) y apunta _%esp_ a dicha regiÃ³n (bien al lÃ­mite inferior, bien al superior, segÃºn la direcciÃ³n en que crece la pila en la arquitectura).

Un kernel, en cambio, es responsable de asignarse su propio _boot stack_ durante el proceso de arranque.

Por su parte, los programas de usuario tambiÃ©n pueden crearse pilas adicionales, por ejemplo para ejecutar varias tareas de manera concurrente.


### Ej: kern2-stack
{: #kern2-stack}

- Lecturas obligatorias

  - BRY2
    - cap. 3: Â§6(**6**{:title="Conditional Move Instructions"})
{:.biblio}

La manera estÃ¡ndar de configurar la pila de arranque del sistema operativo es reservar espacio en el propio binario, esto es: un arreglo en memoria del tamaÃ±o deseado. Puede declararse en un archivo C:

```c
unsigned char kstack[8192];
```

o assembler:

```nasm
.data
kstack:
    .space 8192
```

Normalmente, en x86 se alinea el stack de los procesos de usuario a 16 o 32 bits. Sin embargo, por razones relacionadas â€”como se verÃ¡â€” con memoria virtual, el stack del kernel se suele alinear 4Â KiB:

  - Explicar: Â¿quÃ© significa â€œestar alineadoâ€?

  - Mostrar la sintaxis de C/GCC para alinear a 32 bits el arreglo _kstack_ anterior.

  - Â¿A quÃ© valor se estÃ¡ inicializando _kstack_? Â¿VarÃ­a entre la versiÃ³n C y la versiÃ³n ASM? (Leer la documentaciÃ³n de _as_ sobre la directiva [`.space`][space].)

  - Explicar la diferencia entre las directivas [`.align`][align] y [`.p2align`][p2align] de _as_, y mostrar cÃ³mo alinear el stack del kernel a 4Â KiB usando cada una de ellas.

Otra posibilidad para definir el stack del kernel es hacerlo en alguna otra regiÃ³n de memoria, separada de la imagen del kernel. AsÃ­, si se sobrepasara el espacio reservado, se evitarÃ­a sobreescribir lo que hubiera al continuaciÃ³n (al menos hasta que se puedan introducir otras medidas de protecciÃ³n, como _guard pages_).

Para ello, convendrÃ¡ saber la cantidad de memoria de que dispone la mÃ¡quina. A partir de ahora definiremos que la funciÃ³n _main_ del kernel, `kmain`, reciba un _struct_ con la informaciÃ³n Multiboot proporcionada por el gestor de arranque (ver ejercicio [kern0-gdb](kern0.md#ej-gdb); las definiciones estÃ¡n disponibles en el archivo [multiboot.h](multiboot.h)):

```c
#include "decls.h"
#include "multiboot.h"

void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);
}
```

El archivo _decls.h_, por el momento, tendrÃ­a:

```c
#ifndef KERN2_DECL_H
#define KERN2_DECL_H

#include <stdint.h>

// write.c (funciÃ³n de kern0-vga copiada no-static).
void vga_write(const char *s, int8_t linea, uint8_t color);

#endif
```

Se pide ahora escribir una nueva versiÃ³n del archivo _boot.S_ en que se defina el stack de arranque, asÃ­ como el â€œentry pointâ€ `_start` del kernel. AsÃ­, al saltar a cÃ³digo C, el stack ya estarÃ¡ debidamente configurado:

```nasm
// boot.S

#include "multiboot.h"

#define KSTACK_SIZE 8192

.align 4
multiboot:
    .long MULTIBOOT_HEADER_MAGIC
    .long 0
    .long -(MULTIBOOT_HEADER_MAGIC)

.globl _start
_start:
    // Paso 1: Configurar el stack antes de llamar a kmain.
    movl $0, %ebp
    movl ..., %esp
    push %ebp

    // Paso 2: pasar la informaciÃ³n multiboot a kmain. Si el
    // kernel no arrancÃ³ vÃ­a Multiboot, se debe pasar NULL.
    //
    // Usar una instrucciÃ³n de comparaciÃ³n (TEST o CMP) para
    // comparar con MULTIBOOT_BOOTLOADER_MAGIC, pero no usar
    // un salto a continuaciÃ³n, sino una instrucciÃ³n CMOVcc
    // (copia condicional).
    // ...

    call kmain
halt:
    hlt
    jmp halt

.data
.p2align ...
kstack:
    .space KSTACK_SIZE
```

Finalmente: mostrar en una sesiÃ³n de GDB los valores de _%esp_ y _%eip_ al entrar en `kmain`, asÃ­ como los valores almacenados en el stack en ese momento.

[space]: https://sourceware.org/binutils/docs/as/Space.html
[align]: https://sourceware.org/binutils/docs/as/Align.html
[p2align]: https://sourceware.org/binutils/docs/as/P2align.html
[directiva]: https://sourceware.org/binutils/docs/as/Pseudo-Ops.html


### Ej: kern2-cmdline
{: #ej-cmdline}

- Lecturas obligatorias

  - REES
    - cap. **5**{:title="Pointers and Strings"}

- Lecturas recomendadas

  - BRY2
    - cap. 3: **Â§12**{:title="3.12 Out-of-Bounds Memory References and Buffer Overflow"}
{:.biblio}

En el arranque, el sistema operativo puede recibir parÃ¡metros, al igual que cualquier programa, â€œpor la lÃ­nea de comandosâ€. En el caso de un kernel, la lÃ­nea de comandos es el gestor de arranque, por ejemplo _grub_. Linux permite consultar los parÃ¡metros del arranque en el archivo `/proc/cmdline`:

    $ cat /proc/cmdline
    BOOT_IMAGE=/vmlinuz-4.9.0-3-amd64 root=/dev/sda2 ro

En QEMU, se pueden agregar parÃ¡metros al kernel mediante la opciÃ³n `-append`. Si se aÃ±ade una variable adicional a las [reglas de QEMU](kern0.md#make-qemu) propuestas en el lab anterior:

```make
KERN ?= kern2
BOOT := -kernel $(KERN) $(QEMU_EXTRA)
```

se puede especificar la opciÃ³n directamente al invocar _make:_

    $ make qemu QEMU_EXTRA="-append 'param1=hola param2=adios'"

Ahora que `kmain` recibe un `struct multiboot_info`, se pide expandir _kern2.c_ para imprimir al arrancar los parÃ¡metros recibidos:

```c
#include <string.h>

void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    if (/* mbi->flags indica que hay cmdline */) {
        char buf[256] = "cmdline: ";
        char *cmdline = (void *) mbi->cmdline;
        // AquÃ­ usar strlcat() para concatenar cmdline a buf.
        // ...
        vga_write(buf, 9, 0x07);
    }
}
```

Para manejo de cadenas con _string.h_, se reusa la biblioteca estÃ¡ndar de [Pintos], un kernel educativo de Stanford; en particular:

  - [lib/string.h](string.h)
  - [lib/string.c](string.c)

Estos archivos deben ir en un subdirectorio _lib_, ajustando la variable `SRCS` como corresponda.

Finalmente:

  - Mostrar cÃ³mo implementar la misma concatenaciÃ³n, de manera correcta, usando [`strncat(3)`][strncat].[^nostrncat]

  - Explicar cÃ³mo se comporta [`strlcat(3)`][strlcat] si, errÃ³neamente, se declarase _buf_ con tamaÃ±o 12. Â¿Introduce algÃºn error el cÃ³digo?

  - Compilar el siguiente programa, y explicar por quÃ© se imprimen dos lÃ­neas distintas, en lugar de la misma dos veces:

    ```c
    #include <stdio.h>

    static void printf_sizeof_buf(char buf[256]) {
        printf("sizeof buf = %zu\n", sizeof buf);
    }

    int main(void) {
        char buf[256];
        printf("sizeof buf = %zu\n", sizeof buf);
        printf_sizeof_buf(buf);
    }
    ```

    Revisar, de ser necesario, K&R Â§5.3.[^linus]

[strlcat]: https://www.freebsd.org/cgi/man.cgi?query=strlcat&sektion=3
[strncat]: http://man7.org/linux/man-pages/man3/strcat.3.html
[lkmlarr]: https://lkml.org/lkml/2015/9/3/428 "â€œChrist, people. Learn C, instead of just stringing random characters together until it compiles.â€"

[^nostrncat]: El archivo _string.c_ proporcionado no incluye una implementaciÃ³n de `strncat(3)`. Esta implementaciÃ³n alternativa se puede realizar leyendo la documentaciÃ³n de la funciÃ³n, y probÃ¡ndolo en un programa aparte, en espacio de usuario.

[^linus]: Es por esto que Linus Torvalds, [en su estilo caracterÃ­stico][lkmlarr], recomienda siempre usar `char *buf` y nunca `char buf[â€‹]` en la declaraciÃ³n de una funciÃ³n.

#### Compiler includes
{: #no-host-includes}

En cÃ³digo del kernel no hay acceso a la biblioteca estÃ¡ndar de C, por lo que se debe incluir una implementaciÃ³n de todas las funciones que se invocan.

Para cÃ³digo kernel, el compilador deberÃ­a manejar las directivas _include_ de la siguiente manera:

  1. nunca usar los archivos de la biblioteca estÃ¡ndar de C (p. ej. _string.h_ o _stdlib.h_)

  2. si se necesita, por ejemplo, `#include <string.h>`, se debe buscar en la propia biblioteca del kernel, en este caso el subdirectorio _lib_

  3. los includes estÃ¡ndar de C99 como _stdint.h_ o _stdbool.h_ sÃ­ deberÃ­an estar disponibles (en este caso, los proporciona el mismo compilador y no libc).

La opciÃ³n `-ffreestanding` no es suficiente para conseguir este comportamiento, por lo que se necesitan ajustes adicionales en `CPPFLAGS`. A continuaciÃ³n se muestra cÃ³mo hacerlo en GCC y, mÃ¡s fÃ¡cilmente, usando Clang:

  - Clang

        CPPFLAGS := -nostdlibinc -idirafter lib

  - GCC[^stdlibinc]

        CPPFLAGS := -nostdinc -idirafter lib

        GCC_PATH := /usr/lib/gcc/x86_64-linux-gnu/6 â½Â¹â¾
        CPPFLAGS += -I$(GCC_PATH)/include -I$(GCC_PATH)/include-fixed

        â½Â¹â¾ Consultar la salida de gcc --print-search-dirs.

[Pintos]: http://pintos-os.org/
[nostdinc]: https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdinc
[nostdlibinc]: https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdlibinc

[^stdlibinc]: La opciÃ³n [`-nostdlibinc`][nostdlibinc] de Clang  es precisamente la que se necesita para el kernel; GCC no la tiene, y [`-nostdinc`][nostdinc] implementa el punto 1 sin combinarlo con el 3.


### Ej: kern2-meminfoÂ â˜…
{: #ej-meminfo}

Se desea imprimir durante el arranque la cantidad de memoria fÃ­sica que el sistema reportÃ³ a travÃ©s de Multiboot; por ejemplo:

    $ make qemu QEMU_EXTRA="-append meminfo"
    kern2 loading.............
    cmdline: kern2 meminfo
    Physical memory: 127MiB total (639KiB base, 129920KiB extended)

Se puede cambiar la cantidad de memoria con el parÃ¡metro `-m` de QEMU:

    $ make qemu QEMU_EXTRA="-m 256"
    Physical memory: 255MiB total (639KiB base, 260992KiB extended)

Para imprimir un valor nÃºmero en el buffer VGA se podrÃ­a definir en _write.c_ una funciÃ³n con un prototipo similar a:

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Escribe en â€˜sâ€™ el valor de â€˜valâ€™ en base 10 si su anchura
// es menor que â€˜bufsizeâ€™. En ese caso devuelve true, caso de
// no haber espacio suficiente no hace nada y devuelve false.
bool fmt_int(uint64_t val, char *s, size_t bufsize);
```

AsÃ­, en _kmain_ se puede invocar a _fmt_int_ sobre un buffer temporal, y usar _strlcat_ para componer todas las partes:

```c
char mem[256] = "Physical memory: ";
char tmp[64] = "";

if (fmt_int(NNN, tmp, sizeof tmp)) {
    strlcat(mem, tmp, sizeof mem);
    strlcat(mem, "MiB total", sizeof mem);
}

// ...

vga_write(mem, 10, 0x07);
```

La implementaciÃ³n de _fmt_int_ puede comenzar por calcular la anchura del entero en decimal, y devolver _false_ si no hay espacio suficiente en el buffer recibido:

```c
static size_t int_width(uint64_t val) {
    // ...
}

bool fmt_int(uint64_t val, char *s, size_t bufsize) {
    size_t l = int_width(val);

    if (l >= bufsize)  // Pregunta: Â¿por quÃ© no "l > bufsize"?
        return false;

    s += l;
    // ...
    return true;
}
```

Ayuda adicional:

  - se puede pasar trivialmente de KiB a MiB con una operaciÃ³n de desplazamiento de bits, sin necesidad de divisiÃ³n.

  - quizÃ¡ la funciÃ³n _fmt_int_ sÃ­ necesite realizar una divisiÃ³n y/o operaciÃ³n de mÃ³dulo, en cuyo caso el proceso de compilaciÃ³n quizÃ¡ falle con un error similar a:

        write.c:38: undefined reference to `__umoddi3'
        write.c:40: undefined reference to `__udivdi3'

    Este error estÃ¡ explicado en el documento [Guide to Bare Metal Programming with GCC][107gcc] previamente seÃ±alado, y la soluciÃ³n se reduce a enlazar con _libgcc_. Existen dos maneras de hacerlo:

      1.  Seguir usando directamente _ld_ como enlazador, en cuyo caso hay que solicitar a gcc la ruta completa al archivo _libgcc.a_:

              LIBGCC := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

              $(KERN): $(OBJS)
                      ld -m elf_i386 -Ttext 0x100000 $^ $(LIBGCC) -o $@

      2.  Usar el compilador de C como front-end al enlazador, que es lo que hace _make_ por omisiÃ³n. En ese caso, se debe ajustar la sintaxis de las opciones, y aÃ±adir `-nostdlib`:

              LDLIBS := -lgcc
              LDFLAGS := -m32 -nostdlib -static -Wl,-Ttext=0x100000

              $(KERN): $(OBJS)
                      $(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

          (Esta es, de hecho, la regla implÃ­cita de _make_ para el enlazado.)[^build-id]

[107gcc]: https://cs107e.github.io/guides/gcc/
[^build-id]: Si esta versiÃ³n no arranca, probar a aÃ±adir el flag `-Wl,--build-id=none`, o seguir usando _ld_ directamente.


## Concurrencia cooperativa
{: #cooperative}

Dadas dos o mÃ¡s tareas que se desea ejecutar de manera concurrente en un procesador â€”ï»¿por ejemplo, el cÃ¡lculo de nÃºmeros primos o de los dÃ­gitos de Ï€, o cualquier otra tarea [CPU-bound][cpu-bound]ï»¿â€” lo habitual es asignar a cada un flujo de ejecuciÃ³n separado (ya sean procesos o hilos); y delegar al sistema operativo la implementaciÃ³n de la concurrencia, esto es, la alternancia de ambos flujos.

Suponiendo, para el resto del lab, un sistema con un solo [_core_][core], se presenta ahora la pregunta: Â¿es posible implementar concurrencia de algÃºn tipo sin ayuda del sistema operativo?[^noos]

[^noos]: En este contexto, Â«sin ayuda del sistema operativoÂ» vendrÃ­a a significar: bien en un solo proceso de usuario sin invocar ninguna llamada al sistema relacionada con la planificaciÃ³n; bien en un kernel bÃ¡sico sin reloj configurado.

La respuesta a esta pregunta es una _planificaciÃ³n cooperativa_ en la que dos o mÃ¡s tareas se cedan mutuamente el uso del procesador por un internalo de tiempo. Como se verÃ¡, este tipo de cooperaciÃ³n puede implementarse sin ayuda del sistema operativo; y la implementaciÃ³n mÃ¡s sencilla se consigue proporcionando a cada tarea su propio stack.

[core]: https://unix.stackexchange.com/a/146240
[cpu-bound]: https://en.wikipedia.org/wiki/CPU-bound


### Ej: kern2-task
{: #ej-task}

En el ejercicio [kern2-stack](#kern2-stack) se vio cÃ³mo declarar espacio para stacks adicionales. En este ejercicio se realizarÃ¡n, en dos stacks separados de 4Â KiB, sendas llamadas a la funciÃ³n ya implementada `vga_write()`.

Partiendo de un archivo _kern2.c_ similar a:

```c
#include "decls.h"
#include "multiboot.h"

void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    // A remplazar por una llamada a two_stacks(),
    // definida en stacks.S.
    vga_write("vga_write() from stack1", 12, 0x17);
    vga_write("vga_write() from stack2", 13, 0x90);
}
```

se pide implementar una funciÃ³n `two_stacks()` en assembler que realice las llamadas mostradas en stacks distintos. Se recomienda usar el siguiente segmento de datos:

```nasm
// stacks.S
#define USTACK_SIZE 4096

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
```

y el siguiente esqueleto de implementaciÃ³n:

```nasm
// stacks.S continuado
.text
.globl two_stacks
two_stacks:
        // PreÃ¡mbulo estÃ¡ndar
        push %ebp
        movl %esp, %ebp

        // Registros para apuntar a stack1 y stack2.
        mov $stack1_top, %eax
        mov $stack2_top, ...   // Decidir quÃ© registro usar.

        // Cargar argumentos a ambos stacks en paralelo. Ayuda:
        // usar offsets respecto a %eax ($stack1_top), y lo mismo
        // para el registro usado para stack2_top.
        movl $0x17, ...(%eax)
        movl $0x90, ...(...)

        movl $12, ...
        movl $13, ...

        movl $msg1, ...
        movl $msg2, ...

        // Realizar primera llamada con stack1. Ayuda: usar LEA
        // con el mismo offset que los Ãºltimos MOV para calcular
        // la direcciÃ³n deseada de ESP.
        leal ...(%eax), %esp
        call vga_write

        // Restaurar stack original. Â¿Es %ebp suficiente?
        movl ..., %esp

        // Realizar segunda llamada con stack2.
        leal ...(...), %esp
        call vga_write

        // Restaurar registros callee-saved, si se usaron.
        ...

        leave
        ret
```


### Ej: kern2-exec
{: #ej-exec}

Implementar ahora una funciÃ³n en C que realice la misma tarea que `two_stacks()`, realizando las llamadas de escritura de la siguiente manera:

1.  para la primera llamada, definir una funciÃ³n auxiliar en un nuevo archivo _tasks.S:_

    ```c
    // Realiza una llamada a "entry" sobre el stack proporcionado.
    void task_exec(uintptr_t entry, uintptr_t stack);
    ```

2.  para la segunda llamada, realizar la llamada directamente mediante assembler _inline_.

El cÃ³digo en C puede estar en el mismo archivo _kern2.c_, y debe preparar sus stacks de modo anÃ¡logo a _stacks.S_. Se sugiere el siguiente esqueleto de implementaciÃ³n:

```c
#include "decls.h"
#include "multiboot.h"

#define USTACK_SIZE 4096

void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    two_stacks();
    two_stacks_c();
}

static uint8_t stack1[USTACK_SIZE] __attribute__((aligned(4096)));
static uint8_t stack2[USTACK_SIZE] __attribute__((aligned(4096)));

void two_stacks_c() {
    // Inicializar al *tope* de cada pila.
    uintptr_t *a = ...
    uintptr_t *b = ...

    // Preparar, en stack1, la llamada:
    vga_write("vga_write() from stack1", 15, 0x57);

    // AYUDA 1: se puede usar alguna forma de pre- o post-
    // incremento/decremento, segÃºn corresponda:
    //
    //     *(a++) = ...
    //     *(++a) = ...
    //     *(a--) = ...
    //     *(--a) = ...

    // AYUDA 2: para apuntar a la cadena con el mensaje,
    // es suficiente con el siguiente cast:
    //
    //   ... a ... = (uintptr_t) "vga_write() from stack1";

    // Preparar, en s2, la llamada:
    vga_write("vga_write() from stack2", 16, 0xD0);

    // AYUDA 3: para esta segunda llamada, usar esta forma de
    // asignaciÃ³n alternativa:
    b -= 3;
    b[0] = ...
    b[1] = ...
    b[2] = ...

    // Primera llamada usando task_exec().
    task_exec((uintptr_t) vga_write, (uintptr_t) s1);

    // Segunda llamada con ASM directo. Importante: no
    // olvidar restaurar el valor de %esp al terminar, y
    // compilar con: -fasm -fno-omit-frame-pointer.
    asm("...; call *%1; ..."
        : /* no outputs */
        : "r"(s2), "r"(vga_write));
}
```


### Ej: kern2-regcall
{: #ej-regcall}

Para este ejercicio se aÃ±adirÃ¡ una segunda versiÃ³n de `vga_write()` que toma sus parÃ¡metros directamente por registros:

```c
// decls.h
void __attribute__((regparm(3)))
vga_write2(const char *s, int8_t linea, uint8_t color);
```

Se pide, en primer lugar, leer la documentaciÃ³n de la convenciÃ³n de llamadas [regparm] para implementar la funciÃ³n `vga_write2()` en el archivo _funcs.S:_

```nasm
// tasks.S
.globl vga_write2
vga_write2:
        push %ebp
        movl %esp, %ebp

        // ...
        call vga_write

        leave
        ret
```

A continuaciÃ³n, mostrar con `objdump -d` el cÃ³digo generado por GCC para la siguiente llamada a _vga_write2()_ desde la funciÃ³n principal:

```c
void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    two_stacks();
    two_stacks_c();

    vga_write2("Funciona vga_write2?", 18, 0xE0);
}
```

[regparm]: https://gcc.gnu.org/onlinedocs/gcc/x86-Function-Attributes.html


### Ej: kern2-swap
{: #ej-swap}

En el archivo [contador.c](contador.c) se proporciona una funciÃ³n `contador_yield()` que muestra en el buffer VGA una cuenta desde 0 hasta un nÃºmero pasado como parÃ¡metro. Para simular un retardo entre nÃºmero y nÃºmero, a cada incremento se le aÃ±ade un ciclo _while_ con un nÃºmero alto de iteraciones (controlado por la macro `DELAY`).

AÃ±adir el archivo a las fuentes de _kern2_ y comprobar que se ejecutan dos contadores (uno verde y otro rojo) al invocar a `contador_run()` desde la funciÃ³n principal:

```c
void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    two_stacks();
    two_stacks_c();
    contador_run();  // Nueva llamada ej. kern2-swap.

    vga_write2("Funciona vga_write2?", 18, 0xE0);
}
```

Al final de cada iteraciÃ³n, esto es, de cada incremento, el cÃ³digo del contador invoca a la funciÃ³n `yield()`.[^yield] Esta funciÃ³n simplemente invoca, pasando una variable estÃ¡tica como parÃ¡metro, a la funciÃ³n `task_swap()`, que es el verdadero objetivo de este ejercicio:

```c
// Pone en ejecuciÃ³n la tarea cuyo stack estÃ¡ en â€˜*espâ€™, cuyo
// valor se intercambia por el valor actual de %esp. Guarda y
// restaura todos los callee-called registers.
void task_swap(uintptr_t *esp);
```

En este ejercicio se pide, siguiendo las instrucciones indicadas:

1.  Reescribir la funciÃ³n `contador_run()` para que se ejecute cada contador en un stack separado:

    ```c
    void contador_run() {
        // Configurar stack1 y stack2 con los valores apropiados.
        uintptr_t *a = ...
        uintptr_t *b = ...

        ...

        // Actualizar la variable estÃ¡tica â€˜espâ€™ para que apunte
        // al del segundo contador.
        ...

        // Lanzar el primer contador con task_exec.
        task_exec(...);
    }
    ```

    Consideraciones:

      - la configuraciÃ³n del stack del primer contador, que se ejecuta con `task_exec()`, serÃ¡ muy similar a las realizadas en la funciÃ³n `two_stacks_c()` del ejercicio [kern2-exec](#ej-exec).

      - la configuraciÃ³n del segundo contador es mÃ¡s compleja, y seguramente sea mejor realizarla tras implementar `task_swap()`; pues se debe crear artificialmente el stack tal y como lo hubiera preparado esta funciÃ³n.

    Explicar, para el stack de cada contador, cuÃ¡ntas posiciones se asignan, y quÃ© representa cada una.

2.  Implementar en _tasks.S_ la funciÃ³n `task_swap()`. Como se indicÃ³ arriba, esta funciÃ³n recibe como parÃ¡metro la ubicaciÃ³n en memoria de la variable _esp_ que apunta al stack de la tarea en â€œstand-byâ€. La responsabilidad de esta funciÃ³n es:

      - guardar, en el stack de la tarea actual, los registros que son _callee-saved_

      - cargar en _%esp_ el stack de la nueva tarea, y guardar en la variable _esp_ el valor previo de _%esp_

      - restaurar, desde el nuevo stack, los registros que fueron guardados por una llamada previa a `task_swap()`, y retornar (con la instrucciÃ³n `ret`) a la nueva tarea.

    Para esta funciÃ³n, se recomienda no usar el preÃ¡mbulo, esto es, no  modificar el registro _%ebp_ al entrar en la funciÃ³n.

[^yield]: El verbo _yield_ (en inglÃ©s, ceder el paso) es el tÃ©rmino que se suele usar para indicar que una tarea cede voluntariamente el uso del procesador a otra.


### Ej: kern2-exitÂ â˜…
{: #ej-exit}

En la funciÃ³n `contador_run()` del ejercicio anterior, se configuran ambos contadores con el mismo nÃºmero de iteraciones. Reducir ahora el nÃºmero de iteraciones del _segundo_ contador, y describir quÃ© tipo de error ocurre.

Si la segunda tarea finaliza antes, le corresponde realizar una Ãºltima llamada a `task_swap()` que:

1.  ceda por una vez mÃ¡s el procesador a la primera tarea
2.  anule la variable _esp_, de manera que la primera tarea no ceda mÃ¡s el control, hasta su finalizaciÃ³n

Se podrÃ­a definir una funciÃ³n a tal efecto:

```c
static void exit() {
    uintptr_t *tmp = ...
    esp = ...
    task_swap(...);
}
```

Completar la definiciÃ³n, y realizar una llamada a la misma al finalizar el ciclo principal de `contador_yield()`: ahora deberÃ­a funcionar el caso en el que la segunda tarea termina primero.

A continuaciÃ³n, eliminar la llamada explÃ­cita a `exit()`, y programar su ejecuciÃ³n vÃ­a `contador_run()`, esto es: al preparar el stack del segundo contador.


## Interrupciones: reloj y teclado
{: #interrupts}

Para poder hacer planificaciÃ³n basada en intervalos de tiempo, es necesario tener configurada una interrupciÃ³n de reloj que de manera periodica devuelva el control de la CPU al kernel. AsÃ­, en cada uno de esos instantes el kernel tendrÃ¡ oportunidad de decidir si corresponde cambiar la tarea en ejecuciÃ³n.

El mismo mecanismo de interrupciones permite tambiÃ©n el manejo de dispositivos fÃ­sicos; por ejemplo, el teclado genera una interrupciÃ³n para indicar al sistema operativo que se generÃ³ un evento (por ejemplo, la pulsaciÃ³n de una tecla).

<!--
En esta parte, se implementarÃ¡ un contador en pantalla que muestre, en segundos, el tiempo transcurrido desde el arranque del sistema. Para ello se configurarÃ¡ el reloj de la CPU para generar interrupciones una vez cada 100â€‰ms. En cada interrupciÃ³n, el sistema operativo tendrÃ¡ oportunidad de actualizar el contador, si corresponde.
-->

### Ej: kern2-idt

El mecanismo de interrupciones se describe en detalle en **IA32-3A**, capÃ­tulo 6: _Interrupt and Exception Handling_. El objetivo de este primer ejercicio serÃ¡ la configuraciÃ³n de la tabla de interrupciones _(interrupt descriptor table)_. Para ello, se implementarÃ¡n las siguientes funciones:

```c
// interrupts.c
void idt_init(void);
void idt_install(uint8_t n, void (*handler)(void));

// idt_entry.S
void breakpoint(void);
```

A continuaciÃ³n se proporciona una guÃ­a detallada.


#### Definiciones

Tras leer las secciones 6.1â€“6.5 6.10â€“6.11 de **IA32-3A** y las definiciones del archivo [interrupts.h](interrupts.h), responder:

1.  Â¿CuÃ¡ntos bytes ocupa una entrada en la IDT?

2.  Â¿CuÃ¡ntas entradas como mÃ¡ximo puede albergar la IDT?

3.  Â¿CuÃ¡l es el valor mÃ¡ximo aceptable para el campo _limit_ del registro _IDTR_?

4.  Indicar quÃ© valor exacto tomarÃ¡ el campo _limit_ para una IDT de 64 descriptores solamente.

5.  Consultar la secciÃ³n 6.1 y explicar la diferencia entre interrupciones (Â§6.3) y excepciones (Â§6.4).

#### Variables estÃ¡ticas

Declarar, en un nuevo archivo _interrupts.c_ dos variables globales estÃ¡ticas para el registro _IDTR_ y para la _IDT_ en sÃ­:

```c
#include "decls.h"
#include "interrupts.h"

static ... idtr;
static ... idt[...];
```

A estas variables se accederÃ¡ desde `idt_init()` e `idt_install()`.

#### idt_init()

La funciÃ³n _idt_init()_ debe realizar las siguientes tareas:

  - inicializar los campos _base_ y _limit_ de la variable global _idtr:_

    ```c
    idtr.base = (uintptr_t) ...
    idtr.limit = ...
    ```

  - mediante la instrucciÃ³n LIDT, activar el uso de la IDT configurada (inicialmente vacÃ­a). Se puede usar la instrucciÃ³n:

    ```c
    asm("lidt %0" : : "m"(idtr));
    ```

#### kmain()

Para probar el funcionamiento de las rutinas de interrupciÃ³n, se puede generar una excepciÃ³n por software con la instrucciÃ³n `INT3`. AsÃ­, antes de pasar a configurar el reloj, en la funciÃ³n _kmain()_ se aÃ±adirÃ¡:

```c
void kmain(const multiboot_info_t *mbi) {
    // ...

    two_stacks();
    two_stacks_c();

    // CÃ³digo ejercicio kern2-idt.
    idt_init();   // (a)
    asm("int3");  // (b)

    vga_write2("Funciona vga_write2?", 18, 0xE0);
}
```

Si la implementaciÃ³n de _idt_init()_ es correcta, el kernel deberÃ­a ejecutar la llamada **(a)** con Ã©xito y lanzar un â€œtriple faultâ€ al llegar en **(b)** a la instrucciÃ³n `INT3` (puesto que no se instalÃ³ aÃºn una rutina para manejarla).[^commentint3]

[^commentint3]: Se aconseja dejar la instrucciÃ³n `INT3` comentada hasta que se implemente _idt_init()_ correctamente. Asimismo, para agilizar el desarrollo, se puede dejar comentada la llamada a _contador_run()_.

#### idt_install()

La funciÃ³n _idt_install()_ actualiza en la tabla global `idt` la entrada correspondiente al cÃ³digo de excepciÃ³n pasado como primer argumento. El segundo argumento es la direcciÃ³n de la funciÃ³n que manejarÃ¡ la excepciÃ³n.

En otras palabras, se trata de completar los campos de `idt[n]`:

```c
// Multiboot siempre define "8" como el segmento de cÃ³digo.
// (Ver campo CS en `info registers` de QEMU.)
static const uint8_t KSEG_CODE = 8;

// Identificador de "Interrupt gate de 32 bits" (ver IA32-3A,
// tabla 6-2: IDT Gate Descriptors).
static const uint8_t STS_IG32 = 0xE;

void idt_install(uint8_t n, void (*handler)(void)) {
    uintptr_t addr = (uintptr_t) handler;

    idt[n].rpl = 0;
    idt[n].type = STS_IG32;
    idt[n].segment = KSEG_CODE;

    idt[n].off_15_0 = addr & ...
    idt[n].off_31_16 = addr >> ...

    idt[n].present = 1;
}
```

Una vez implementada, desde _idt_init()_ se deberÃ¡ instalar el manejador para la excepciÃ³n _breakpoint_ (`T_BRKPT` definida en _interrupts.h);_ el manejador serÃ¡ la funciÃ³n `breakpoint()`, cuya implementaciÃ³n inicial tambiÃ©n se incluye:

```c
void idt_init() {
    // (1) Instalar manejadores ("interrupt service routines").
    idt_install(T_BRKPT, breakpoint);

    // (2) Configurar ubicaciÃ³n de la IDT.
    idtr.base = (uintptr_t) ...
    idtr.limit = ...

    // (3) Activar IDT.
    asm("lidt %0" : : "m"(idtr));
}
```

Definir tambiÃ©n, en un nuevo archivo _idt_entry.S_, una primera versiÃ³n de la funciÃ³n _breakpoint()_ con una Ãºnica instrucciÃ³n `IRET`:

```nasm
.globl breakpoint
breakpoint:
        // Manejador mÃ­nimo.
        iret
```

Con esta primera definiciÃ³n, _kern2_ deberÃ­a arrancar sin problemas, llegando hasta la ejecuciÃ³n de `vga_write2()`.

### Ej: kern2-isr
{: #ej-isr}

Cuando ocurre una excepciÃ³n, la CPU inmediatamente invoca al manejador configurado, esto es, justo tras la instrucciÃ³n original que produjo la excepciÃ³n.

Un manejador de interrupciones (en inglÃ©s _interrupt service routine)_ es, salvo por algunos detalles, una funciÃ³n comÃºn sin parÃ¡metros.

Las diferencias principales son:

  - desde el punto de vista del manejador, en `(%esp)` se encuentra la direcciÃ³n de retorno; la CPU, no obstante, aÃ±ade alguna informaciÃ³n adicional a la pila, tal y como se verÃ¡ en la sesiÃ³n de GDB que se solicita a continuaciÃ³n.

  - a diferencia del ejercicio [kern2-swap](#ej-swap), donde la propia tarea solicitaba un relevo con llamando explÃ­citamente a _task_swap()_, ahora la tarea es desalojada de la CPU sin previo aviso. Desde el punto de vista de esa tarea original, la ejecuciÃ³n del manejador deberÃ­a ser â€œinvisibleâ€, esto es, que el manejador deberÃ¡ restaurar el estado exacto de la CPU antes de devolver el control de la CPU a la tarea anterior.

En este ejercicio se pide:

1.  Una sesiÃ³n de GDB que muestre el estado de la pila antes, durante y despuÃ©s de la ejecuciÃ³n del manejador.

2.  Una implementaciÃ³n ampliada de `breakpoint()` que imprima un mensaje en el buffer VGA.

#### SesiÃ³n de GDB

Se debe seguir el mismo guiÃ³n **dos veces**:

  - versiÃ³n A: usando esta implementaciÃ³n aumentada del manejador:

    ```nasm
    .globl breakpoint
    breakpoint:
            nop
            test %eax, %eax
            iret
    ```

  - versiÃ³n B: con el mismo manejador, pero cambiando la instrucciÃ³n `IRET` por una instrucciÃ³n `RET`.

Los pasos a seguir son:

0.  Activar la impresiÃ³n de la siguiente instrucciÃ³n ejecutando:

        display/i $pc

1.  Poner un breakpoint en la funciÃ³n _idt_init()_ y, una vez dentro, finalizar su ejecuciÃ³n con el comando de GDB `finish`. Mostrar, en ese momento, las siguientes instrucciones (con el comando `disas` o `x/10i $pc`): la ejecuciÃ³n deberÃ­a haberse detenido en la misma instrucciÃ³n `int3`.[^nobp] Mostrar:

      - el valor de _%esp_ (`print $esp`)
      - el valor de _(%esp)_ (`x/xw $esp`)
      - el valor de `$cs`
      - el resultado de `print $eflags` y `print/x $eflags`

2.  Ejecutar la instrucciÃ³n `int3` mediante el comando de GDB `stepi`. La ejecuciÃ³n deberÃ­a saltar directamente a la instrucciÃ³n `test %eax, %eax`; en ese momento:

      - imprimir el valor de _%esp;_ Â¿cuÃ¡ntas posiciones avanzÃ³?
      - si avanzÃ³ _N_ posiciones, mostrar (con `x/Nwx $sp`) los _N_ valores correspondientes
      - mostrar el valor de `$eflags`

    Responder: Â¿quÃ© representa cada valor? (Ver IA32-3A, Â§6.12: _Exception and Interrupt Handling_.)

3.  Avanzar una instrucciÃ³n mÃ¡s con `stepi`, ejecutando la instrucciÃ³n `TEST`. Mostrar, como anteriormente, el valor del registro _EFLAGS_ (en dos formatos distintos, usando `print` y `print/x`).

4.  Avanzar, por Ãºltima vez, una instrucciÃ³n, de manera que se ejecute `IRET` para la sesiÃ³n A, y `RET` para la sesiÃ³n B. Mostrar, de nuevo lo pedido que en el punto 1; y explicar cualquier diferencia entre ambas versiones A y B.

[^nobp]: Para este ejercicio, se debe evitar poner un breakpoint en la instrucciÃ³n `int3` directamente.

#### VersiÃ³n final de breakpoint()

La versiÃ³n de _breakpoint()_ a entregar simplemente realiza, desde _idt_entry.S,_ la siguiente llamada:

```c
vga_write2("Hello, breakpoint", 14, 0xB0);
```

siguiendo la estructura:

```nasm
.globl breakpoint
breakpoint:
        // (1) Guardar registros.
        ...
        // (2) Preparar argumentos de la llamada.
        ...
        // (3) Invocar a vga_write2()
        call vga_write2
        // (4) Restaurar registros.
        ...
        // (5) Finalizar ejecuciÃ³n del manejador.
        iret

.data
breakpoint_msg:
        .asciz "Hello, breakpoint"
```

Como se explicÃ³ al comienzo del ejercicio, la ejecuciÃ³n del manejador debe resultar invisible para la tarea original (en este caso, la funciÃ³n _kmain)_. Por tanto, se debe asegurar que todos los registros volvieron a su valor original antes de ejecutar `iret`.

Incluir las respuestas a las siguientes cuestiones:

1.  Para cada una de las siguientes maneras de guardar/restaurar registros en _breakpoint_, indicar si es correcto (en el sentido de hacer su ejecuciÃ³n â€œinvisibleâ€), y justificar por quÃ©:

    ```nasm
    // OpciÃ³n A.
    breakpoint:
        pusha
        ...
        call vga_write2
        popa
        iret

    // OpciÃ³n B.
    breakpoint:
        push %eax
        push %edx
        push %ecx
        ...
        call vga_write2
        pop %ecx
        pop %edx
        pop %eax
        iret

    // OpciÃ³n C.
    breakpoint:
        push %ebx
        push %esi
        push %edi
        ...
        call vga_write2
        pop %edi
        pop %esi
        pop %ebx
        iret
    ```

2.  Responder de nuevo la pregunta anterior, sustituyendo en el cÃ³digo `vga_write2` por `vga_write`. (Nota: el cÃ³digo representado con `...` corresponderÃ­a a la nueva convenciÃ³n de llamadas.)

3.  Si la ejecuciÃ³n del manejador debe ser enteramente invisible Â¿no serÃ­a necesario guardar y restaurar el registro _EFLAGS_ a mano? Â¿Por quÃ©?

4.  Â¿En quÃ© stack se ejecuta la funciÃ³n _vga_write()?_


### Ej: kern2-irq
{: #ej-irq}

Los cÃ³digos de excepciÃ³n 0 a 31 forman parte de la definiciÃ³n de la arquitectura x86 (**IA32-3A**, Â§6.3.1); su significado es fijo. Los cÃ³digos 32 a 255 estÃ¡n disponibles para su uso bien por el sistema operativo, bien por dispositivos fÃ­sicos del sistema.

En la arquitectura PC tradicional, la coordinaciÃ³n entre kernel y dispositivos fÃ­sicos emplea un _Programmable Interrupt Controller_ (PIC) que, entre otras cosas, permite al sistema operativo:

  - detectar los dispositivos presentes
  - configurar aspectos de algunos de los dispositivos
  - asignar a cada uno de ellos un cÃ³digo de interrupciÃ³n propio

En particular, el procesador i386 dispone de dos PIC 8259 capaces de manejar 8 fuentes de interrupciÃ³n cada uno; en este lab, usaremos solamente el primero de ellos.

Al arrancar la mÃ¡quina, no se genera interrupciÃ³n alguna hasta que se habilita su uso con la instrucciÃ³n `STI`. En ese momento, por ejemplo, si se hace uso del teclado, se generarÃ­a una interrupciÃ³n. En _kern2_, esto se harÃ¡ desde una nueva funciÃ³n _irq_init():_

```c
// interrupts.c
void irq_init() {
    // (1) Redefinir cÃ³digos para IRQs.
    ...

    // (2) Instalar manejadores.
    ...

    // (3) Habilitar interrupciones.
    asm("sti");
}

// kern2.c
void kmain(const multiboot_info_t *mbi) {
    // ...
    idt_init();
    irq_init();   // Nueva funciÃ³n.
    asm("int3");

    vga_write2("Funciona vga_write2?", 18, 0xE0);
}
```

Si no se completan los puntos **(1)** y **(2)** de la funciÃ³n _irq_init()_, al ejecutar _kern2_ se producirÃ¡ de nuevo un â€œtriple faultâ€ porque â€”entre otras cosasâ€” el reloj del sistema comienza a lanzar interrupciones para las que no hay un manejador. Se deberÃ¡ instalar uno que al menos comunique al PIC que se procesÃ³ la interrupciÃ³n:

```c
void irq_init() {
    irq_remap();

    idt_install(T_TIMER, ack_irq);
    idt_install(T_KEYBOARD, ack_irq);

    asm("sti");
}
```

definiendo, en _idt_entry.S:_

```nasm
#define PIC1 0x20
#define ACK_IRQ 0x20

.globl ack_irq:
ack_irq:
        // Indicar que se manejÃ³ la interrupciÃ³n.
        movl $ACK_IRQ, %eax
        outb %al, $PIC1
        iret
```

Observaciones:

  - se instala el mismo manejador para el teclado, de manera que tampoco ocurran errores si se presiona una tecla en la ventana de QEMU.

  - por omisiÃ³n, el modelo PIC 8259 usa el cÃ³digo 0 para el timer, y el cÃ³digo 1 para el teclado; pero estos cÃ³digos de excepciÃ³n estÃ¡n en conflicto con los cÃ³digos propios de la arquitectura x86 (el cÃ³digo 0, por ejemplo, corresponde a un error en una operaciÃ³n `DIV`)

  - el propÃ³sito de la funciÃ³n `irq_remap()`, cuyo cÃ³digo se incluye a continuaciÃ³n, es desplazar los cÃ³digos de interrupciÃ³n PIC de tal manera que comiencen en 32, y no en 0.

  - las constantes `T_TIMER` y `T_KEYBOARD` se definieron en el archivo _interrupts.h_ con valores 32 y 33, respectivamente.

    ```c
    #define outb(port, data) \
            asm("outb %b0,%w1" : : "a"(data), "d"(port));

    static void irq_remap() {
        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0x0);
        outb(0xA1, 0x0);
    }
    ```

La necesidad de escribir los manejadores en assembler surge de la obligaciÃ³n de usar `iret` y no `ret` para finalizar su ejecuciÃ³n; pero se puede, desde assembler, invocar a rutinas escritas en C, por ejemplo:

```c
// handlers.c
#include "decls.h"

static unsigned ticks;

void timer() {
    if (++ticks == 15) {
        vga_write("Transcurrieron 15 ticks", 20, 0x07);
    }
}
```

Para poder invocar a esta funciÃ³n:

  - definir en _idt_entry.S_ un â€œtrampolÃ­nâ€ en assembler:

    ```nasm
    .globl timer_asm
    timer_asm:
            // Guardar registros.
            ...
            call timer
            // Restaurar registros.
            ...
            jmp ack_irq
    ```

  - en _interrupts.c_, instalar `timer_asm` en lugar de `ack_irq` para `T_TIMER`.


### Ej: kern2-div
{: #ej-div}

En este Ãºltimo ejercicio se estudia el subtipo particular de excepciÃ³n llamado _fault_ (ver **IA32-3A** Â§6.5 y Â§6.6).

Cuando ocurre una interrupciÃ³n, o una excepciÃ³n de tipo _trap_, se ejecuta inmediatamente el manejador y, una vez finalizado Ã©ste, se reanuda la ejecuciÃ³n de la tarea original en la _siguiente_ instrucciÃ³n. Por el contrario, para excepciones de tipo _fault_, se vuelve a intentar la _misma_ instrucciÃ³n.[^segfault]

[^segfault]: Esta funcionalidad es principalmente Ãºtil cuando comienza a haber procesos no privilegiados de usuario, ya que se da oportunidad al kernel de decidir quÃ© hacer si un programa muestra un comportamiento anÃ³malo (por ejemplo, intentar escribir en una regiÃ³n de memoria de sÃ³lo lectura).

La instrucciÃ³n `DIV` genera una excepciÃ³n _Divide Error_ (cÃ³digo numÃ©rico 0) cuando, entre otros casos, el divisor es 0. Como la divisiÃ³n por 0 tambiÃ©n es comportamiento no definido en C, usaremos directamente inline assembly para generar la excepciÃ³n desde _kmain()_.

Se pide:

1.  Modificar la funciÃ³n _kmain()_ como se indica, y verificar que _kern2_ arranca e imprime sus mensajes de manera correcta:

    ```c
    void kmain(const multiboot_info_t *mbi) {
        int8_t linea;
        uint8_t color;

        // ...

        idt_init();
        irq_init();

        asm("div %4"
            : "=a"(linea), "=c"(color)
            : "0"(18), "1"(0xE0), "b"(1), "d"(0));

        vga_write2("Funciona vga_write2?", linea, color);
    }

    ```

2.  Explicar el funcionamiento exacto de la lÃ­nea `asm(...)` del punto anterior:

      - Â¿quÃ© cÃ³mputo se estÃ¡ realizando?
      - Â¿de dÃ³nde sale el valor de la variable _color_?
      - Â¿por quÃ© se da valor 0 a _%edx?_

3.  Asignar a _%ebx_ el valor 0 en lugar de 1, y comprobar que se genera una triple falla al ejecutar el kernel.

4.  Definir en _write.c_ una nueva variante de escritura:

    ```c
    void __attribute__((regparm(2)))
    vga_write_cyan(const char *s, int8_t linea) {
        vga_write(s, linea, 0xB0);
    }
    ```

5.  Escribir, en _idt_entry.S_, un manejador `divzero` a instalar desde _idt_init():_

    ```c
    idt_install(T_DIVIDE, divzero);
    ```

    El manejador debe, primero, incrementar el valor de _%ebx_, de manera que cuando se reintente la instrucciÃ³n, Ã©sta tenga Ã©xito.

    Asimismo, realizar desde assembly la llamada:

    ```c
    vga_write_cyan("Se divide por ++ebx", 17);
    ```

    Requerimiento: no usar `pusha`/`popa`; guardar y restaurar el mÃ­nimo nÃºmero de registros necesarios.

### Ej: kern2-kbdÂ â˜…
{: #ej-kbd}

En el archivo [handlers.c](handlers.c) se incluyen un par de ejemplos de manejo del timer y del teclado. Sobre el cÃ³digo del teclado se pide:

  - manejar la tecla Shift, y emitir caracteres en mayÃºsculas cuando estÃ© presionada.

DocumentaciÃ³n [de ejemplo](http://www.osdever.net/bkerndev/Docs/keyboard.htm).

<!--
### Ej: kern2-timesyncÂ â˜…
{: #ej-timesync}
-->

<!--
## Desalojo
{: #preempt}

### Ej: kern2-stepÂ â˜…
{: #ej-step}

En el ejercicio siguiente [kern2-swtch](#ej-swtch) se implementarÃ¡ planificaciÃ³n cooperativa propiamente dicha, en la que dos tareas se ceden el control de la CPU _mÃºltiples vecesï»¿;_ esto es: el flujo de ejecuciÃ³n cicla entre ambas. Antes, en este ejercicio se simularÃ¡ una especie de desalojo, una sola vez, durante la ejecuciÃ³n de una funciÃ³n.


## Enlazado y archivos ELF
{: #elf}

### Ej: kern2-elf
{: #ej-elf}

### Ej: kern2-load
{: #ej-load}

### Ej: kern2-mon_kerninfo
{: #ej-kerninfo}

### Ej: kern2-mbrÂ â˜…
{: #ej-mbr}
-->

{% include anchors.html %}
{% include footnotes.html %}
