// kernel/gdt.c - Flat 32-bit GDT (real descriptor table, loaded with lgdt)
#include "gdt.h"

// Three entries: null, flat code (0x08), flat data (0x10).
static gdt_entry_t gdt_entries[3];

// gdt_flush: loads the table with lgdt, far-jumps to reload CS,
// then reloads the data segments. Plain x86 instructions, no mocks.
static void gdt_flush(void) {
    gdt_ptr_t gp;
    gp.limit = sizeof(gdt_entries) - 1;
    gp.base  = (uint32_t)&gdt_entries[0];

    __asm__ volatile (
        "lgdt %0\n\t"
        "ljmp $0x08, $1f\n\t"  /* reload CS via far jump (flat code) */
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        : : "m"(gp) : "memory"
    );
}

// Set one descriptor.
static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity) {
    gdt_entries[i].limit_low   = limit & 0xFFFF;
    gdt_entries[i].base_low    = base & 0xFFFF;
    gdt_entries[i].base_mid    = (base >> 16) & 0xFF;
    gdt_entries[i].access      = access;
    gdt_entries[i].granularity = (limit >> 16) & 0x0F;
    gdt_entries[i].granularity |= granularity & 0xF0;
    gdt_entries[i].base_high   = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    // Null descriptor
    gdt_set_entry(0, 0, 0, 0, 0);
    // Flat 32-bit code: base 0, limit 4 GiB, 4K granularity,
    // present, DPL 0, non-conforming, readable, accessed
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);
    // Flat 32-bit data: base 0, limit 4 GiB, 4K granularity,
    // present, DPL 0, expand-up, writable, accessed
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);
    gdt_flush();
}
