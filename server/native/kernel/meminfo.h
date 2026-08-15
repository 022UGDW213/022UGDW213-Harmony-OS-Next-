#ifndef HARMONY_MEMINFO_H
#define HARMONY_MEMINFO_H

#include <stdint.h>

// Parse the Multiboot 1 info structure (real detection: mem_lower/mem_upper
// plus the BIOS mmap entries) and print the detected RAM to VGA + serial.
void meminfo_init(uint32_t mb_info_addr);

uint32_t meminfo_total_kb(void);       // mem_lower + mem_upper (KiB)
uint32_t meminfo_available_mb(void);   // sum of available mmap entries (MiB)
uint32_t meminfo_mmap_entries(void);   // number of mmap entries parsed

#endif // HARMONY_MEMINFO_H
