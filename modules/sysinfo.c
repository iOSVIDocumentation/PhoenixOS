#include "sysinfo.h"
#include "ff.h"

extern char __bss_end__; /* символ линкера: конец статических данных */

void sysinfo_ram_kb(uint32_t *total_kb, uint32_t *free_kb) {
    register uint32_t sp;
    __asm volatile ("mov %0, sp" : "=r" (sp));
    *total_kb = 520; /* SRAM RP2350 */
    uint32_t freeb = sp - (uint32_t)&__bss_end__;
    *free_kb = freeb / 1024;
}

void sysinfo_sd_kb(uint32_t *total_kb, uint32_t *free_kb) {
    FATFS *fs = 0;
    DWORD nclst = 0;
    *total_kb = 0; *free_kb = 0;
    if (f_getfree("0:", &nclst, &fs) != FR_OK || !fs) return;
    uint32_t csize_kb = ((uint32_t)fs->csize * 512) / 1024;
    *free_kb = (uint32_t)nclst * csize_kb;
    *total_kb = (uint32_t)(fs->n_fatent - 2) * csize_kb;
}
