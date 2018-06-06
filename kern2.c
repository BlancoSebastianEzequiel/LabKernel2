#include "decls.h"
#include "multiboot.h"
#include "lib/string.h"
#include "lib/stddef.h"
//------------------------------------------------------------------------------
//  PRINT NUM
//------------------------------------------------------------------------------
void print_num(const size_t num, int8_t linea, uint8_t color) {
    char size_char[256];
    int decenas = num/10;
    int unidades = num-(decenas*10);
    size_char[0] = (char) (decenas + 48);
    size_char[1] = (char) (unidades + 48);
    vga_write(size_char, linea, color);
}
//------------------------------------------------------------------------------
//  KMAIN
//------------------------------------------------------------------------------
void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);
    if (mbi->flags || 1) {
        char buf[256] = "cmdline: ";
        // char *cmdline = (void *) mbi->cmdline;
        char cmdline[256] = "hola!!";
        strlcat(buf, cmdline, strlen(buf)+strlen(cmdline)+1);
        vga_write(buf, 9, 0x07);
    }
    while (1) __asm__("hlt");
}
//------------------------------------------------------------------------------
