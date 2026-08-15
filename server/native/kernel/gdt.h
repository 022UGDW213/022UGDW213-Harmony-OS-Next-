#ifndef HARMONY_GDT_H
#define HARMONY_GDT_H

#include <stdint.h>

// Flat 32-bit GDT selectors
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10

// Structure describing a single GDT entry (64-bit segment descriptor).
typedef struct {
    uint16_t limit_low;   // Lower 16 bits of the limit
    uint16_t base_low;    // Lower 16 bits of the base
    uint8_t  base_mid;    // Next 8 bits of the base
    uint8_t  access;      // Access flags (present, DPL, type)
    uint8_t  granularity; // Granularity + upper 4 bits of the limit
    uint8_t  base_high;   // Upper 8 bits of the base
} __attribute__((packed)) gdt_entry_t;

// GDT pointer passed to `lgdt`.
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);

#endif // HARMONY_GDT_H
