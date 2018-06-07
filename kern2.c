#include "decls.h"
#include "multiboot.h"
#include "lib/string.h"
#include "lib/stddef.h"
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
        char *cmdline = (void *) mbi->cmdline;
        strlcat(buf, cmdline, strlen(buf) + strlen(cmdline) + 1);
        vga_write(buf, 9, 0x07);
    }

    char mem[256] = "Physical memory: ";
    char tmp[64] = "";

    if (fmt_int(mbi->cmdline, tmp, sizeof tmp)) {
        strlcat(mem, tmp, sizeof mem);
        strlcat(mem, "MiB total", sizeof mem);
    }
    vga_write(mem, 10, 0x07);

    while (1) __asm__("hlt");
}
//------------------------------------------------------------------------------
