// kernel/meminfo.c - real RAM detection from the Multiboot 1 info struct.
// The bootloader (QEMU -kernel direct boot) fills mem_lower/mem_upper and
// the BIOS memory map (mmap_addr/mmap_length) that we parse here.
#include "meminfo.h"
#include "../boot/multiboot.h"
#include "kprintf.h"

// Multiboot 1 info structure (flags: bit0 = basic meminfo, bit6 = mmap)
struct mb1_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
};

// Multiboot 1 mmap entry (packed, 24 bytes + size field)
struct mb1_mmap_entry {
    uint32_t size;      // size of this entry, NOT including the size field
    uint32_t base_lo;
    uint32_t base_hi;
    uint32_t len_lo;
    uint32_t len_hi;
    uint32_t type;      // 1 = available RAM
} __attribute__((packed));

#define MB1_FLAG_MEMINFO 0x1
#define MB1_FLAG_MMAP    0x40

static uint32_t detected_total_kb = 0;
static uint32_t detected_avail_mb = 0;
static uint32_t detected_entries = 0;

uint32_t meminfo_total_kb(void)     { return detected_total_kb; }
uint32_t meminfo_available_mb(void) { return detected_avail_mb; }
uint32_t meminfo_mmap_entries(void) { return detected_entries; }

void meminfo_init(uint32_t mb_info_addr) {
    const struct mb1_info* info = (const struct mb1_info*)mb_info_addr;

    kprintf("[MEM] Multiboot info @ %x flags=%x\n", mb_info_addr, info->flags);

    if (info->flags & MB1_FLAG_MEMINFO) {
        uint32_t lower_kb = info->mem_lower;   // KB usable below 1 MiB
        uint32_t upper_kb = info->mem_upper;   // KB usable above 1 MiB
        detected_total_kb = lower_kb + upper_kb;
        kprintf("[MEM] mem_lower=%u KiB, mem_upper=%u KiB -> total=%u KiB (%u MiB)\n",
                lower_kb, upper_kb, detected_total_kb, detected_total_kb >> 10);
    } else {
        kprintf("[MEM] bootloader provided no basic meminfo\n");
    }

    if (info->flags & MB1_FLAG_MMAP) {
        const struct mb1_mmap_entry* entry =
            (const struct mb1_mmap_entry*)info->mmap_addr;
        const struct mb1_mmap_entry* end =
            (const struct mb1_mmap_entry*)(info->mmap_addr + info->mmap_length);

        kprintf("[MEM] BIOS memory map (%u bytes):\n", info->mmap_length);
        while (entry < end) {
            uint64_t base = ((uint64_t)entry->base_hi << 32) | entry->base_lo;
            uint64_t len  = ((uint64_t)entry->len_hi  << 32) | entry->len_lo;
            const char* type_str = (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
                                   ? "available" : "reserved";

            detected_entries++;
            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                // len < 4 GiB in all realistic cases: print MiB from low word
                if (entry->len_hi == 0) {
                    detected_avail_mb += (entry->len_lo >> 20);
                    kprintf("[MEM]   base=%x%08x len=%u KiB (%u MiB) type=%s\n",
                            entry->base_hi, entry->base_lo,
                            entry->len_lo >> 10, entry->len_lo >> 20, type_str);
                } else {
                    kprintf("[MEM]   base=%x%08x len=%x%08x (>=4GiB) type=%s\n",
                            entry->base_hi, entry->base_lo,
                            entry->len_hi, entry->len_lo, type_str);
                }
            } else {
                kprintf("[MEM]   base=%x%08x len=%u KiB type=%s\n",
                        entry->base_hi, entry->base_lo,
                        entry->len_lo >> 10, type_str);
            }
            (void)base; (void)len;  // kept for future PMM use
            entry = (const struct mb1_mmap_entry*)((uint32_t)entry +
                                                   entry->size + 4);
        }
        kprintf("[MEM] %u mmap entries, available RAM %u MiB\n",
                detected_entries, detected_avail_mb);
    } else {
        kprintf("[MEM] bootloader provided no mmap\n");
    }
}
