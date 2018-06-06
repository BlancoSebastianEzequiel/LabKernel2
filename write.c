#include "decls.h"
#define VGABUF ((volatile char *) 0xb8000)
#define MAX_X 80
#define MAX_Y 25
//------------------------------------------------------------------------------
// VGA WRITE
//------------------------------------------------------------------------------
// Se escribe la cadena en la línea indicada de la pantalla
// (si linea es menor que cero, se empieza a contar desde abajo).
void vga_write(const char *s, int8_t linea, uint8_t color) {
    int vgaPsoition;
    volatile char *buf = VGABUF;
    if (linea < 0) {
        vgaPsoition = MAX_Y + linea;
    } else {
        vgaPsoition = linea;
    }
    int pos = 0;
    buf += 2*vgaPsoition*MAX_X;
    while (s[pos] != '\0') {
        *buf++ = s[pos];
        *buf++ = color;
        pos++;
    }
}