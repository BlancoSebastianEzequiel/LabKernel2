# Un kernel con reloj y tres tareas

Material de apoyo:

  - [decls.h](decls.h)
  - [interrupts.h](interrupts.h)
  - [multiboot.h](multiboot.h)

## Indice
* [Creación de stacks en el kernel](##Creación-de-stacks-en-el-kernel)
  * [Ej: kern2-stack](###kern2-stack)
  * [Ej: kern2-cmdline](###kern2-cmdline)
  * [Ej: kern2-meminfo ★](###kern2-meminfo)
* [Concurrencia cooperativa](##Concurrencia-cooperativa)
  * [Ej: kern2-task](###kern2-task)
  * [Ej: kern2-exec](###kern2-exec)
  * [Ej: kern2-regcall](###kern2-regcall)
  * [Ej: kern2-swap](###kern2-swap)
  * [Ej: kern2-exit ★](###kern2-exit)
* [Interrupciones: reloj y teclado](##Interrupciones:-reloj-y-teclado)
  * [Ej: kern2-idt](###kern2-idt)
  * [Ej: kern2-isr](###kern2-isr)
  * [Ej: kern2-irq](###kern2-irq)
  * [Ej: kern2-div](###kern2-div)
  * [Ej: kern2-kbd ★](###kern2-kbd-★)



## Creacion de stacks en el kernel

Cuando un programa se ejecuta, normalmente es el sistema operativo quien le configura el stack de manera adecuada, esto es: reserva un espacio en memoria (a menudo 4 u 8Â KiB) y apunta _%esp_ a dicha region (bien al lÃ­mite inferior, bien al superior, segun la direccion en que crece la pila en la arquitectura).

Un kernel, en cambio, es responsable de asignarse su propio _boot stack_ durante el proceso de arranque.

Por su parte, los programas de usuario tambiÃ©n pueden crearse pilas adicionales, por ejemplo para ejecutar varias tareas de manera concurrente.


### Ej: kern2-stack

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

  - Explicar: ¿que significa estar alineado?

  - Mostrar la sintaxis de C/GCC para alinear a 32 bits el arreglo _kstack_ anterior.

  - ¿A que valor se esta inicializando _kstack_? ¿Vari­a entre la version C y la version ASM? (Leer la documentaciÃ³n de _as_ sobre la directiva [`.space`][space].)

  - Explicar la diferencia entre las directivas [`.align`][align] y [`.p2align`][p2align] de _as_, y mostrar cÃ³mo alinear el stack del kernel a 4Â KiB usando cada una de ellas.

Otra posibilidad para definir el stack del kernel es hacerlo en alguna otra regiÃ³n de memoria, separada de la imagen del kernel. AsÃ­, si se sobrepasara el espacio reservado, se evitaria sobreescribir lo que hubiera al continuaciÃ³n (al menos hasta que se puedan introducir otras medidas de protecciÃ³n, como _guard pages_).

Para ello, convendrÃ¡ saber la cantidad de memoria de que dispone la mÃ¡quina. A partir de ahora definiremos que la funcion _main_ del kernel, `kmain`, reciba un _struct_ con la informacion Multiboot proporcionada por el gestor de arranque (ver ejercicio [kern0-gdb](kern0.md#ej-gdb); las definiciones estan disponibles en el archivo [multiboot.h](multiboot.h)):

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

Se pide ahora escribir una nueva version del archivo _boot.S_ en que se defina el stack de arranque, asÃi como el "entry point" `_start` del kernel. AsÃi, al saltar a codigo C, el stack ya estara debidamente configurado:

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

    // Paso 2: pasar la informacion multiboot a kmain. Si el
    // kernel no arrancÃ³ vÃ­a Multiboot, se debe pasar NULL.
    //
    // Usar una instruccion de comparacion (TEST o CMP) para
    // comparar con MULTIBOOT_BOOTLOADER_MAGIC, pero no usar
    // un salto a continuacion, sino una instruccion CMOVcc
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

Finalmente: mostrar en una sesion de GDB los valores de _%esp_ y _%eip_ al entrar en `kmain`, asi­ como los valores almacenados en el stack en ese momento.

[space]: https://sourceware.org/binutils/docs/as/Space.html
[align]: https://sourceware.org/binutils/docs/as/Align.html
[p2align]: https://sourceware.org/binutils/docs/as/P2align.html
[directiva]: https://sourceware.org/binutils/docs/as/Pseudo-Ops.html


### Ej: kern2-cmdline

- Lecturas obligatorias

  - REES
    - cap. **5**{:title="Pointers and Strings"}

- Lecturas recomendadas

  - BRY2
    - cap. 3: **Â§12**{:title="3.12 Out-of-Bounds Memory References and Buffer Overflow"}
{:.biblio}

En el arranque, el sistema operativo puede recibir parametros, al igual que cualquier programa, "por la linea de comandos". En el caso de un kernel, la lÃ­nea de comandos es el gestor de arranque, por ejemplo _grub_. Linux permite consultar los parametros del arranque en el archivo `/proc/cmdline`:

    $ cat /proc/cmdline
    BOOT_IMAGE=/vmlinuz-4.9.0-3-amd64 root=/dev/sda2 ro

En QEMU, se pueden agregar parÃ¡metros al kernel mediante la opcion `-append`. Si se aÃ±ade una variable adicional a las [reglas de QEMU](kern0.md#make-qemu) propuestas en el lab anterior:

```make
KERN ?= kern2
BOOT := -kernel $(KERN) $(QEMU_EXTRA)
```

se puede especificar la opciÃ³n directamente al invocar _make:_

    $ make qemu QEMU_EXTRA="-append 'param1=hola param2=adios'"

Ahora que `kmain` recibe un `struct multiboot_info`, se pide expandir _kern2.c_ para imprimir al arrancar los parametros recibidos:

```c
#include <string.h>

void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);

    if (/* mbi->flags indica que hay cmdline */) {
        char buf[256] = "cmdline: ";
        char *cmdline = (void *) mbi->cmdline;
        // Aqui usar strlcat() para concatenar cmdline a buf.
        // ...
        vga_write(buf, 9, 0x07);
    }
}
```

Para manejo de cadenas con _string.h_, se reusa la biblioteca estandar de [Pintos], un kernel educativo de Stanford; en particular:

  - [lib/string.h](string.h)
  - [lib/string.c](string.c)

Estos archivos deben ir en un subdirectorio _lib_, ajustando la variable `SRCS` como corresponda.

Finalmente:

  - Mostrar como implementar la misma concatenacion, de manera correcta, usando [`strncat(3)`][strncat].[^nostrncat]

  - Explicar como se comporta [`strlcat(3)`][strlcat] si, errÃ³neamente, se declarase _buf_ con tamaÃ±o 12. Â¿Introduce algun error el codigo?

  - Compilar el siguiente programa, y explicar por que se imprimen dos li­neas distintas, en lugar de la misma dos veces:

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

[^linus]: Es por esto que Linus Torvalds, [en su estilo caracterÃ­stico][lkmlarr], recomienda siempre usar `char *buf` y nunca `char buf[â€‹]` en la declaracion de una funcion.

#### Compiler includes

En codigo del kernel no hay acceso a la biblioteca estandar de C, por lo que se debe incluir una implementacion de todas las funciones que se invocan.

Para codigo kernel, el compilador debera manejar las directivas _include_ de la siguiente manera:

  1. nunca usar los archivos de la biblioteca estandar de C (p. ej. _string.h_ o _stdlib.h_)

  2. si se necesita, por ejemplo, `#include <string.h>`, se debe buscar en la propia biblioteca del kernel, en este caso el subdirectorio _lib_

  3. los includes estandar de C99 como _stdint.h_ o _stdbool.h_ si deberan estar disponibles (en este caso, los proporciona el mismo compilador y no libc).

La opcion `-ffreestanding` no es suficiente para conseguir este comportamiento, por lo que se necesitan ajustes adicionales en `CPPFLAGS`. A continuacion se muestra como hacerlo en GCC y, mas facilmente, usando Clang:

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

[^stdlibinc]: La opcion [`-nostdlibinc`][nostdlibinc] de Clang  es precisamente la que se necesita para el kernel; GCC no la tiene, y [`-nostdinc`][nostdinc] implementa el punto 1 sin combinarlo con el 3.


### Ej: kern2-meminfo ★

Se desea imprimir durante el arranque la cantidad de memoria fisica que el sistema reporta a traves de Multiboot; por ejemplo:

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

Asi­, en _kmain_ se puede invocar a _fmt_int_ sobre un buffer temporal, y usar _strlcat_ para componer todas las partes:

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

La implementacion de _fmt_int_ puede comenzar por calcular la anchura del entero en decimal, y devolver _false_ si no hay espacio suficiente en el buffer recibido:

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

  - se puede pasar trivialmente de KiB a MiB con una operacion de desplazamiento de bits, sin necesidad de division.

  - quiza la funcion _fmt_int_ sÃ­ necesite realizar una division y/o operacion de modulo, en cuyo caso el proceso de compilacion quiza falle con un error similar a:

        write.c:38: undefined reference to `__umoddi3'
        write.c:40: undefined reference to `__udivdi3'

    Este error esta explicado en el documento [Guide to Bare Metal Programming with GCC][107gcc] previamente señalado, y la solucion se reduce a enlazar con _libgcc_. Existen dos maneras de hacerlo:

      1.  Seguir usando directamente _ld_ como enlazador, en cuyo caso hay que solicitar a gcc la ruta completa al archivo _libgcc.a_:

              LIBGCC := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

              $(KERN): $(OBJS)
                      ld -m elf_i386 -Ttext 0x100000 $^ $(LIBGCC) -o $@

      2.  Usar el compilador de C como front-end al enlazador, que es lo que hace _make_ por omision. En ese caso, se debe ajustar la sintaxis de las opciones, y añadir `-nostdlib`:

              LDLIBS := -lgcc
              LDFLAGS := -m32 -nostdlib -static -Wl,-Ttext=0x100000

              $(KERN): $(OBJS)
                      $(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

          (Esta es, de hecho, la regla implÃ­cita de _make_ para el enlazado.)[^build-id]

[107gcc]: https://cs107e.github.io/guides/gcc/
[^build-id]: Si esta version no arranca, probar a aÃ±adir el flag `-Wl,--build-id=none`, o seguir usando _ld_ directamente.


## Concurrencia cooperativa

Dadas dos o mas tareas que se desea ejecutar de manera concurrente en un procesador â€”ï»¿por ejemplo, el cÃ¡lculo de nÃºmeros primos o de los dÃ­gitos de Ï€, o cualquier otra tarea [CPU-bound][cpu-bound]ï»¿â€” lo habitual es asignar a cada un flujo de ejecuciÃ³n separado (ya sean procesos o hilos); y delegar al sistema operativo la implementaciÃ³n de la concurrencia, esto es, la alternancia de ambos flujos.

Suponiendo, para el resto del lab, un sistema con un solo [_core_][core], se presenta ahora la pregunta: Â¿es posible implementar concurrencia de algÃºn tipo sin ayuda del sistema operativo?[^noos]

[^noos]: En este contexto, Â«sin ayuda del sistema operativoÂ» vendrÃ­a a significar: bien en un solo proceso de usuario sin invocar ninguna llamada al sistema relacionada con la planificaciÃ³n; bien en un kernel bÃ¡sico sin reloj configurado.

La respuesta a esta pregunta es una _planificacion cooperativa_ en la que dos o mÃ¡s tareas se cedan mutuamente el uso del procesador por un internalo de tiempo. Como se verÃ¡, este tipo de cooperaciÃ³n puede implementarse sin ayuda del sistema operativo; y la implementaciÃ³n mÃ¡s sencilla se consigue proporcionando a cada tarea su propio stack.

[core]: https://unix.stackexchange.com/a/146240
[cpu-bound]: https://en.wikipedia.org/wiki/CPU-bound


### Ej: kern2-task

En el ejercicio [kern2-stack](#kern2-stack) se vio como declarar espacio para stacks adicionales. En este ejercicio se realizaran, en dos stacks separados de 4Â KiB, sendas llamadas a la funcion ya implementada `vga_write()`.

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

se pide implementar una funcion `two_stacks()` en assembler que realice las llamadas mostradas en stacks distintos. Se recomienda usar el siguiente segmento de datos:

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

y el siguiente esqueleto de implementacion:

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
        // la direccion deseada de ESP.
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

Implementar ahora una funcion en C que realice la misma tarea que `two_stacks()`, realizando las llamadas de escritura de la siguiente manera:

1.  para la primera llamada, definir una funcion auxiliar en un nuevo archivo _tasks.S:_

    ```c
    // Realiza una llamada a "entry" sobre el stack proporcionado.
    void task_exec(uintptr_t entry, uintptr_t stack);
    ```

2.  para la segunda llamada, realizar la llamada directamente mediante assembler _inline_.

El codigo en C puede estar en el mismo archivo _kern2.c_, y debe preparar sus stacks de modo analogo a _stacks.S_. Se sugiere el siguiente esqueleto de implementacion:

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
    // incremento/decremento, segun corresponda:
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

Para este ejercicio se añadira una segunda version de `vga_write()` que toma sus parametros directamente por registros:

```c
// decls.h
void __attribute__((regparm(3)))
vga_write2(const char *s, int8_t linea, uint8_t color);
```

Se pide, en primer lugar, leer la documentacion de la convencion de llamadas [regparm] para implementar la funcion `vga_write2()` en el archivo _funcs.S:_

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

A continuacion, mostrar con `objdump -d` el codigo generado por GCC para la siguiente llamada a _vga_write2()_ desde la funcion principal:

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

En el archivo [contador.c](contador.c) se proporciona una funcion `contador_yield()` que muestra en el buffer VGA una cuenta desde 0 hasta un numero pasado como parametro. Para simular un retardo entre numero y numero, a cada incremento se le aÃ±ade un ciclo _while_ con un numero alto de iteraciones (controlado por la macro `DELAY`).

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

Al final de cada iteracion, esto es, de cada incremento, el codigo del contador invoca a la funcion `yield()`.[^yield] Esta funcion simplemente invoca, pasando una variable estÃ¡tica como parÃ¡metro, a la funciÃ³n `task_swap()`, que es el verdadero objetivo de este ejercicio:

```c
// Pone en ejecucion la tarea cuyo stack esta en â€˜*espâ€™, cuyo
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

      - la configuracion del stack del primer contador, que se ejecuta con `task_exec()`, serÃ¡ muy similar a las realizadas en la funcion `two_stacks_c()` del ejercicio [kern2-exec](#ej-exec).

      - la configuracion del segundo contador es mas compleja, y seguramente sea mejor realizarla tras implementar `task_swap()`; pues se debe crear artificialmente el stack tal y como lo hubiera preparado esta funciÃ³n.

    Explicar, para el stack de cada contador, cuantas posiciones se asignan, y que representa cada una.

2.  Implementar en _tasks.S_ la funcion `task_swap()`. Como se indicÃ³ arriba, esta funcion recibe como parÃ¡metro la ubicacion en memoria de la variable _esp_ que apunta al stack de la tarea en â€œstand-byâ€. La responsabilidad de esta funciÃ³n es:

      - guardar, en el stack de la tarea actual, los registros que son _callee-saved_

      - cargar en _%esp_ el stack de la nueva tarea, y guardar en la variable _esp_ el valor previo de _%esp_

      - restaurar, desde el nuevo stack, los registros que fueron guardados por una llamada previa a `task_swap()`, y retornar (con la instrucciÃ³n `ret`) a la nueva tarea.

    Para esta funcion, se recomienda no usar el preambulo, esto es, no  modificar el registro _%ebp_ al entrar en la funcion.

[^yield]: El verbo _yield_ (en ingles, ceder el paso) es el tÃ©rmino que se suele usar para indicar que una tarea cede voluntariamente el uso del procesador a otra.


### Ej: kern2-exit ★

En la funcion `contador_run()` del ejercicio anterior, se configuran ambos contadores con el mismo numero de iteraciones. Reducir ahora el nÃºmero de iteraciones del _segundo_ contador, y describir que tipo de error ocurre.

Si la segunda tarea finaliza antes, le corresponde realizar una ultima llamada a `task_swap()` que:

1.  ceda por una vez mas el procesador a la primera tarea
2.  anule la variable _esp_, de manera que la primera tarea no ceda mas el control, hasta su finalizaciÃ³n

Se podra definir una funcion a tal efecto:

```c
static void exit() {
    uintptr_t *tmp = ...
    esp = ...
    task_swap(...);
}
```

Completar la definicion, y realizar una llamada a la misma al finalizar el ciclo principal de `contador_yield()`: ahora debera funcionar el caso en el que la segunda tarea termina primero.

A continuacion, eliminar la llamada explÃ­cita a `exit()`, y programar su ejecucion vÃ­a `contador_run()`, esto es: al preparar el stack del segundo contador.


## Interrupciones: reloj y teclado

Para poder hacer planificacion basada en intervalos de tiempo, es necesario tener configurada una interrupcion de reloj que de manera periodica devuelva el control de la CPU al kernel. Asi, en cada uno de esos instantes el kernel tendra oportunidad de decidir si corresponde cambiar la tarea en ejecucion.

El mismo mecanismo de interrupciones permite tambiÃ©n el manejo de dispositivos fi­sicos; por ejemplo, el teclado genera una interrupcion para indicar al sistema operativo que se genero un evento (por ejemplo, la pulsaciÃ³n de una tecla).

<!--
En esta parte, se implementara un contador en pantalla que muestre, en segundos, el tiempo transcurrido desde el arranque del sistema. Para ello se configurara el reloj de la CPU para generar interrupciones una vez cada 100â€‰ms. En cada interrupciÃ³n, el sistema operativo tendrÃ¡ oportunidad de actualizar el contador, si corresponde.
-->

### Ej: kern2-idt

El mecanismo de interrupciones se describe en detalle en **IA32-3A**, capÃ­tulo 6: _Interrupt and Exception Handling_. El objetivo de este primer ejercicio sera la configuracion de la tabla de interrupciones _(interrupt descriptor table)_. Para ello, se implementaran las siguientes funciones:

```c
// interrupts.c
void idt_init(void);
void idt_install(uint8_t n, void (*handler)(void));

// idt_entry.S
void breakpoint(void);
```

A continuacion se proporciona una guÃ­a detallada.


#### Definiciones

Tras leer las secciones 6.1â€“6.5 6.10â€“6.11 de **IA32-3A** y las definiciones del archivo [interrupts.h](interrupts.h), responder:

1.  ¿Cuantos bytes ocupa una entrada en la IDT?

2.  ¿Cuantas entradas como maximo puede albergar la IDT?

3.  ¿Cual es el valor maximo aceptable para el campo _limit_ del registro _IDTR_?

4.  Indicar que valor exacto tomara el campo _limit_ para una IDT de 64 descriptores solamente.

5.  Consultar la seccion 6.1 y explicar la diferencia entre interrupciones (Â§6.3) y excepciones (Â§6.4).

#### Variables estÃ¡ticas

Declarar, en un nuevo archivo _interrupts.c_ dos variables globales estÃ¡ticas para el registro _IDTR_ y para la _IDT_ en sÃ­:

```c
#include "decls.h"
#include "interrupts.h"

static ... idtr;
static ... idt[...];
```

A estas variables se accedera desde `idt_init()` e `idt_install()`.

#### idt_init()

La funcion _idt_init()_ debe realizar las siguientes tareas:

  - inicializar los campos _base_ y _limit_ de la variable global _idtr:_

    ```c
    idtr.base = (uintptr_t) ...
    idtr.limit = ...
    ```

  - mediante la instruccion LIDT, activar el uso de la IDT configurada (inicialmente vacia). Se puede usar la instruccion:

    ```c
    asm("lidt %0" : : "m"(idtr));
    ```

#### kmain()

Para probar el funcionamiento de las rutinas de interrupcion, se puede generar una excepcion por software con la instruccion `INT3`. AsÃ­, antes de pasar a configurar el reloj, en la funciÃ³n _kmain()_ se añadira:

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

Si la implementaciÃ³n de _idt_init()_ es correcta, el kernel deberÃ­a ejecutar la llamada **(a)** con exito y lanzar un â€œtriple faultâ€ al llegar en **(b)** a la instruccion `INT3` (puesto que no se instalÃ³ aÃºn una rutina para manejarla).[^commentint3]

[^commentint3]: Se aconseja dejar la instruccion `INT3` comentada hasta que se implemente _idt_init()_ correctamente. Asimismo, para agilizar el desarrollo, se puede dejar comentada la llamada a _contador_run()_.

#### idt_install()

La funcion _idt_install()_ actualiza en la tabla global `idt` la entrada correspondiente al codigo de excepcion pasado como primer argumento. El segundo argumento es la direccion de la funcion que manejara la excepciÃ³n.

En otras palabras, se trata de completar los campos de `idt[n]`:

```c
// Multiboot siempre define "8" como el segmento de codigo.
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

Una vez implementada, desde _idt_init()_ se debera instalar el manejador para la excepcion _breakpoint_ (`T_BRKPT` definida en _interrupts.h);_ el manejador sera la funcion `breakpoint()`, cuya implementacion inicial tambien se incluye:

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

Definir tambien, en un nuevo archivo _idt_entry.S_, una primera version de la funcion _breakpoint()_ con una unica instruccion `IRET`:

```nasm
.globl breakpoint
breakpoint:
        // Manejador mÃ­nimo.
        iret
```

Con esta primera definicion, _kern2_ deberÃ­a arrancar sin problemas, llegando hasta la ejecucion de `vga_write2()`.

### Ej: kern2-isr

Cuando ocurre una excepcion, la CPU inmediatamente invoca al manejador configurado, esto es, justo tras la instruccion original que produjo la excepcion.

Un manejador de interrupciones (en ingles _interrupt service routine)_ es, salvo por algunos detalles, una funcion comÃºn sin parametros.

Las diferencias principales son:

  - desde el punto de vista del manejador, en `(%esp)` se encuentra la direccion de retorno; la CPU, no obstante, añade alguna informacion adicional a la pila, tal y como se vera en la sesion de GDB que se solicita a continuacion.

  - a diferencia del ejercicio [kern2-swap](#ej-swap), donde la propia tarea solicitaba un relevo con llamando explÃ­citamente a _task_swap()_, ahora la tarea es desalojada de la CPU sin previo aviso. Desde el punto de vista de esa tarea original, la ejecucion del manejador debera ser â€œinvisibleâ€, esto es, que el manejador debera restaurar el estado exacto de la CPU antes de devolver el control de la CPU a la tarea anterior.

En este ejercicio se pide:

1.  Una sesion de GDB que muestre el estado de la pila antes, durante y despues de la ejecucion del manejador.

2.  Una implementacion ampliada de `breakpoint()` que imprima un mensaje en el buffer VGA.

#### Sesion de GDB

Se debe seguir el mismo guian **dos veces**:

  - version A: usando esta implementacion aumentada del manejador:

    ```nasm
    .globl breakpoint
    breakpoint:
            nop
            test %eax, %eax
            iret
    ```

  - version B: con el mismo manejador, pero cambiando la instruccion `IRET` por una instruccion `RET`.

Los pasos a seguir son:

0.  Activar la impresion de la siguiente instrucciÃ³n ejecutando:

        display/i $pc

1.  Poner un breakpoint en la funcion _idt_init()_ y, una vez dentro, finalizar su ejecucion con el comando de GDB `finish`. Mostrar, en ese momento, las siguientes instrucciones (con el comando `disas` o `x/10i $pc`): la ejecucion deber­a haberse detenido en la misma instruccion `int3`.[^nobp] Mostrar:

      - el valor de _%esp_ (`print $esp`)
      - el valor de _(%esp)_ (`x/xw $esp`)
      - el valor de `$cs`
      - el resultado de `print $eflags` y `print/x $eflags`

2.  Ejecutar la instruccion `int3` mediante el comando de GDB `stepi`. La ejecucion deber­a saltar directamente a la instruccion `test %eax, %eax`; en ese momento:

      - imprimir el valor de _%esp;_ ¿cuantas posiciones avanzo?
      - si avanzo _N_ posiciones, mostrar (con `x/Nwx $sp`) los _N_ valores correspondientes
      - mostrar el valor de `$eflags`

    Responder: ¿que representa cada valor? (Ver IA32-3A, Â§6.12: _Exception and Interrupt Handling_.)

3.  Avanzar una instruccion mas con `stepi`, ejecutando la instruccion `TEST`. Mostrar, como anteriormente, el valor del registro _EFLAGS_ (en dos formatos distintos, usando `print` y `print/x`).

4.  Avanzar, por ultima vez, una instruccion, de manera que se ejecute `IRET` para la sesion A, y `RET` para la sesion B. Mostrar, de nuevo lo pedido que en el punto 1; y explicar cualquier diferencia entre ambas versiones A y B.

[^nobp]: Para este ejercicio, se debe evitar poner un breakpoint en la instruccion `int3` directamente.

#### Version final de breakpoint()

La version de _breakpoint()_ a entregar simplemente realiza, desde _idt_entry.S,_ la siguiente llamada:

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

Como se explico al comienzo del ejercicio, la ejecucion del manejador debe resultar invisible para la tarea original (en este caso, la funcion _kmain)_. Por tanto, se debe asegurar que todos los registros volvieron a su valor original antes de ejecutar `iret`.

Incluir las respuestas a las siguientes cuestiones:

1.  Para cada una de las siguientes maneras de guardar/restaurar registros en _breakpoint_, indicar si es correcto (en el sentido de hacer su ejecucion â€œinvisibleâ€), y justificar por quÃ©:

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

2.  Responder de nuevo la pregunta anterior, sustituyendo en el codigo `vga_write2` por `vga_write`. (Nota: el codigo representado con `...` correspondera a la nueva convencion de llamadas.)

3.  Si la ejecucion del manejador debe ser enteramente invisible ¿no ser­a necesario guardar y restaurar el registro _EFLAGS_ a mano? ¿Por que?

4.  ¿En que stack se ejecuta la funcion _vga_write()?_


### Ej: kern2-irq

Los codigos de excepcion 0 a 31 forman parte de la definicion de la arquitectura x86 (**IA32-3A**, Â§6.3.1); su significado es fijo. Los codigos 32 a 255 estan disponibles para su uso bien por el sistema operativo, bien por dispositivos fÃ­sicos del sistema.

En la arquitectura PC tradicional, la coordinaciÃ³n entre kernel y dispositivos fisicos emplea un _Programmable Interrupt Controller_ (PIC) que, entre otras cosas, permite al sistema operativo:

  - detectar los dispositivos presentes
  - configurar aspectos de algunos de los dispositivos
  - asignar a cada uno de ellos un cÃ³digo de interrupcion propio

En particular, el procesador i386 dispone de dos PIC 8259 capaces de manejar 8 fuentes de interrupcion cada uno; en este lab, usaremos solamente el primero de ellos.

Al arrancar la maquina, no se genera interrupcion alguna hasta que se habilita su uso con la instruccion `STI`. En ese momento, por ejemplo, si se hace uso del teclado, se generara una interrupcion. En _kern2_, esto se harÃ¡ desde una nueva funcion _irq_init():_

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

Si no se completan los puntos **(1)** y **(2)** de la funcion _irq_init()_, al ejecutar _kern2_ se producira de nuevo un â€œtriple faultâ€ porque â€”entre otras cosasâ€” el reloj del sistema comienza a lanzar interrupciones para las que no hay un manejador. Se debera instalar uno que al menos comunique al PIC que se procesÃ³ la interrupcion:

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

  - por omision, el modelo PIC 8259 usa el codigo 0 para el timer, y el cÃ³digo 1 para el teclado; pero estos codigos de excepcion estan en conflicto con los cÃ³digos propios de la arquitectura x86 (el cÃ³digo 0, por ejemplo, corresponde a un error en una operaciÃ³n `DIV`)

  - el proposito de la funcion `irq_remap()`, cuyo codigo se incluye a continuacion, es desplazar los codigos de interrupcion PIC de tal manera que comiencen en 32, y no en 0.

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

En este ultimo ejercicio se estudia el subtipo particular de excepciÃ³n llamado _fault_ (ver **IA32-3A** Â§6.5 y Â§6.6).

Cuando ocurre una interrupcion, o una excepcion de tipo _trap_, se ejecuta inmediatamente el manejador y, una vez finalizado Ã©ste, se reanuda la ejecuciÃ³n de la tarea original en la _siguiente_ instruccion. Por el contrario, para excepciones de tipo _fault_, se vuelve a intentar la _misma_ instrucciÃ³n.[^segfault]

[^segfault]: Esta funcionalidad es principalmente util cuando comienza a haber procesos no privilegiados de usuario, ya que se da oportunidad al kernel de decidir que hacer si un programa muestra un comportamiento anomalo (por ejemplo, intentar escribir en una region de memoria de sÃ³lo lectura).

La instruccion `DIV` genera una excepcion _Divide Error_ (codigo numÃ©rico 0) cuando, entre otros casos, el divisor es 0. Como la division por 0 tambien es comportamiento no definido en C, usaremos directamente inline assembly para generar la excepcion desde _kmain()_.

Se pide:

1.  Modificar la funcion _kmain()_ como se indica, y verificar que _kern2_ arranca e imprime sus mensajes de manera correcta:

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

      - ¿que computo se esta realizando?
      - ¿de donde sale el valor de la variable _color_?
      - ¿por que se da valor 0 a _%edx?_

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

    El manejador debe, primero, incrementar el valor de _%ebx_, de manera que cuando se reintente la instruccion, esta tenga exito.

    Asimismo, realizar desde assembly la llamada:

    ```c
    vga_write_cyan("Se divide por ++ebx", 17);
    ```

    Requerimiento: no usar `pusha`/`popa`; guardar y restaurar el mi­nimo numero de registros necesarios.

### Ej: kern2-kbd ★

En el archivo [handlers.c](handlers.c) se incluyen un par de ejemplos de manejo del timer y del teclado. Sobre el codigo del teclado se pide:

  - manejar la tecla Shift, y emitir caracteres en mayusculas cuando esta presionada.

Documentacion [de ejemplo](http://www.osdever.net/bkerndev/Docs/keyboard.htm).

<!--
### Ej: kern2-timesyncÂ â˜…
{: #ej-timesync}
-->

<!--
## Desalojo
{: #preempt}

### Ej: kern2-stepÂ â˜…
{: #ej-step}

En el ejercicio siguiente [kern2-swtch](#ej-swtch) se implementara planificacion cooperativa propiamente dicha, en la que dos tareas se ceden el control de la CPU _multiples vecesï»¿;_ esto es: el flujo de ejecuciÃ³n cicla entre ambas. Antes, en este ejercicio se simulara una especie de desalojo, una sola vez, durante la ejecucion de una funcion.


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
