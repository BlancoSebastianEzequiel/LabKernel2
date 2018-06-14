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
//------------------------------------------------------------------------------
// INT WIDTH
//------------------------------------------------------------------------------
static size_t int_width(uint64_t val) {
    uint64_t n = val;
    size_t digits = 0;
    while(n > 0) {
        digits++;
        n = n / 10;
    }
    return digits;
}
//------------------------------------------------------------------------------
// FMT INT
//------------------------------------------------------------------------------
// Escribe en ‘s’ el valor de ‘val’ en base 10 si su anchura
// es menor que ‘bufsize’. En ese caso devuelve true, caso de
// no haber espacio suficiente no hace nada y devuelve false.
bool fmt_int(uint64_t val, char *s, size_t bufsize) {
    if (val == 0) {
        s[0] = (char) 48;
        s[1] = '\0';
        return true;
    }
    size_t l = int_width(val);
    if (l >= bufsize)  // Pregunta: ¿por qué no "l > bufsize"?
        return false;
    uint64_t n = val;
    size_t pos = l;
    s[pos] = '\0';
    while(n > 0) {
        pos--;
        s[pos] = (char) ((n % 10) + 48);
        n = n / 10;
    }
    return true;
}
//------------------------------------------------------------------------------
