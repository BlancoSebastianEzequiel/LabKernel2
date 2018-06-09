#include "decls.h"
#include "multiboot.h"
#include "lib/string.h"
#include "lib/stddef.h"
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
    uint32_t memSize = mbi->mem_upper - mbi->mem_lower;
    memSize = (memSize >> 10) + 1;

    if (fmt_int(memSize, tmp, sizeof tmp)) {
        strlcat(mem, tmp, sizeof mem);
        strlcat(mem, "MiB total (", sizeof mem);
    }
    if (fmt_int(mbi->mem_lower, tmp, sizeof tmp)) {
        strlcat(mem, tmp, sizeof mem);
        strlcat(mem, " base, ", sizeof mem);
    }
    if (fmt_int(mbi->mem_upper, tmp, sizeof tmp)) {
        strlcat(mem, tmp, sizeof mem);
        strlcat(mem, "KiB extended)", sizeof mem);
    }
    vga_write(mem, 10, 0x07);

    while (1) asm("hlt");
}
//------------------------------------------------------------------------------
