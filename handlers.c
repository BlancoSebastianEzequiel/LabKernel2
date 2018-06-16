#include "decls.h"

static unsigned ticks;
//------------------------------------------------------------------------------
// TIMER
//------------------------------------------------------------------------------
void timer() {
    if (++ticks == 15) {
        vga_write("Transcurrieron 15 ticks", 20, 0x07);
    }
}
//------------------------------------------------------------------------------

/**
 * Mapa de "scancodes" a caracteres ASCII en un teclado QWERTY.
 */
static char klayout[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8',             // 0-9
    '9', '0', 0,   0,   0,   0,   'q', 'w', 'e', 'r',             // 10-19
    't', 'y', 'u', 'i', 'o', 'p', 0,   0,   0,   0,               // 20-29
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0,   0,          // 30-40
    0,   0,   0,   'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.'};  // 41-52

static const uint8_t KBD_PORT = 0x60;

static bool use_upper(uint8_t code) {
    // return false;
    static bool shift_pressed;

    bool released = code & 0x80;
    code = code & ~0x80;

    if (code == 42 || code == 54) {
        shift_pressed = !released;
    }

    return shift_pressed;
}

/**
 * Handler para el teclado (IRQ1).
 *
 * Imprime la letra correspondiente por pantalla.
 */
void keyboard() {
    uint8_t code;
    static char chars[81];
    static uint8_t idx = 0;

    asm volatile("inb %1,%0" : "=a"(code) : "n"(KBD_PORT));

    int8_t upper_shift = use_upper(code) ? -32 : 0;

    if (code >= sizeof(klayout) || !klayout[code])
        return;

    if (idx == 80) {
        while (idx--)
            chars[idx] = ' ';
    }

    chars[idx++] = klayout[code] + upper_shift;
    vga_write(chars, 19, 0x0A);
}
