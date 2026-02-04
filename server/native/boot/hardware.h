#ifndef HARMONY_BOOT_HARDWARE_H
#define HARMONY_BOOT_HARDWARE_H

#include <stdint.h>
#include <stddef.h>

// Simulated Hardware Info
typedef struct {
    char cpu_vendor[13];
    uint32_t cpu_family;
    uint32_t cpu_model;
    uint32_t total_memory_mb;
    uint32_t reserved_memory_mb;
    uint32_t available_memory_mb;
    int has_fpu;
    int has_apic;
} system_info_t;

// API
void hardware_detect(void);
const system_info_t* hardware_get_info(void);
void hardware_print_info(void);

#endif // HARMONY_BOOT_HARDWARE_H
