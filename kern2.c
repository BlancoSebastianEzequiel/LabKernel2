#include "decls.h"
#include "multiboot.h"
void kmain(const multiboot_info_t *mbi) {
    vga_write("kern2 loading.............", 8, 0x70);
    while (1) __asm__("hlt");
}