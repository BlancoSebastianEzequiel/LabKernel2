#include "decls.h"
#include "interrupts.h"

#define IDTSIZE 256

#define LOW_MASK 0xFFFF
#define DESP 16

#define outb(port, data) asm("outb %b0,%w1" : : "a"(data), "d"(port));

static struct IDTR idtr;
static struct Gate idt[IDTSIZE];

// Multiboot siempre define "8" como el segmento de código.
// (Ver campo CS en `info registers` de QEMU.)
static const uint8_t KSEG_CODE = 8;

// Identificador de "Interrupt gate de 32 bits" (ver IA32-3A,
// tabla 6-2: IDT Gate Descriptors).
static const uint8_t STS_IG32 = 0xE;
//------------------------------------------------------------------------------
// IDT INIT
//------------------------------------------------------------------------------
void idt_init(void){

	// (1) Instalar manejadores ("interrupt service routines").
    idt_install(T_BRKPT, breakpoint);
    idt_install(T_DIVIDE, divzero); // Ej: kern2-div

	idtr.base = (uintptr_t) idt;
	idtr.limit = (sizeof(idt) - 1);

	asm("lidt %0" : : "m"(idtr));
}
//------------------------------------------------------------------------------
// IDT INSTALL
//------------------------------------------------------------------------------
void idt_install(uint8_t n, void (*handler)(void)) {
    uintptr_t addr = (uintptr_t) handler;

    idt[n].rpl = 0;
    idt[n].type = STS_IG32;
    idt[n].segment = KSEG_CODE;

    idt[n].off_15_0 = addr & LOW_MASK;
    idt[n].off_31_16 = addr >> DESP;

    idt[n].present = 1;
}
//------------------------------------------------------------------------------
// IRQ REAMP
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// IRQ INIT
//------------------------------------------------------------------------------
void irq_init() {
    // (1) Redefinir códigos para IRQs.
    irq_remap();

    // (2) Instalar manejadores.
    idt_install(T_TIMER, timer_asm);
    idt_install(T_KEYBOARD, ack_irq);

    // (3) Habilitar interrupciones.
    asm("sti");
}
//------------------------------------------------------------------------------