#include "decls.h"
#include "interrupts.h"

#define IDTLIMIT 0xFF
#define IDTBASE 0x00000000

#define LOW_MASK 0xFFFF
#define DESP 16

static struct IDTR idtr;
static struct Gate idt[256];

// Multiboot siempre define "8" como el segmento de código.
// (Ver campo CS en `info registers` de QEMU.)
static const uint8_t KSEG_CODE = 8;

// Identificador de "Interrupt gate de 32 bits" (ver IA32-3A,
// tabla 6-2: IDT Gate Descriptors).
static const uint8_t STS_IG32 = 0xE;

void idt_init(void){

	// (1) Instalar manejadores ("interrupt service routines").
    idt_install(T_BRKPT, breakpoint);

	idtr.base = (uintptr_t) IDTBASE;
	idtr.limit = IDTLIMIT;

	asm("lidt %0" : : "m"(idtr));
}

void idt_install(uint8_t n, void (*handler)(void)) {
    uintptr_t addr = (uintptr_t) handler;

    idt[n].rpl = 0;
    idt[n].type = STS_IG32;
    idt[n].segment = KSEG_CODE;

    idt[n].off_15_0 = addr & LOW_MASK;
    idt[n].off_31_16 = addr >> DESP;

    idt[n].present = 1;
}