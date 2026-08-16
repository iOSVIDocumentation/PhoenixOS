#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>

void sysinfo_ram_kb(uint32_t *total_kb, uint32_t *free_kb);
void sysinfo_sd_kb(uint32_t *total_kb, uint32_t *free_kb);

/* Долгий подсчёт свободного места: вызывать ДО watchdog_enable (на буте) */
void sysinfo_sd_scan(void);

#endif
