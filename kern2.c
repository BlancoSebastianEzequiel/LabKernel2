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
//  DIGITS QUANTITY
//------------------------------------------------------------------------------
size_t digitsQuantity(const uint32_t num) {
    uint32_t n = num;
    size_t digits = 0;
    while(n > 0) {
        digits++;
        n = n / 10;
    }
    return digits;
}
//------------------------------------------------------------------------------
//  UINT32_T TO STRING
//------------------------------------------------------------------------------
void uint32_tToString(const uint32_t num, char* buf, size_t maxSize) {
    uint32_t r;
    size_t digits = digitsQuantity(num);
    if (digits > maxSize) return;
    uint32_t n = num;
    size_t pos = digits;
    buf[pos] = '\0';
    while(n > 0) {
        pos--;
        r = n % 10;
        buf[pos] = (char) (r + 48);
        n = n / 10;
    }
}
//------------------------------------------------------------------------------
//  KMAIN
//------------------------------------------------------------------------------
void kmain(const multiboot_info_t *mbi) {
    if (mbi == NULL) vga_write("mbi == NULL", 10, 0x70);
    vga_write("kern2 loading.............", 8, 0x70);
    if (mbi->flags & MULTIBOOT_INFO_CMDLINE) {
        char buf[256] = "cmdline: ";
        char *cmdline = (void*) mbi->cmdline;
        strlcat(buf, cmdline, strlen(buf)+strlen(cmdline)+1);
        vga_write(buf, 9, 0x07);
    }
    while (1) __asm__("hlt");
}
//------------------------------------------------------------------------------
