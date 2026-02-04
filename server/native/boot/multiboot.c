#include "multiboot.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"

void multiboot_parse(unsigned long addr) {
    struct multiboot_tag *tag;
    unsigned long end;

    // addr points to total size of tags
    unsigned long total_size = *(unsigned long *)addr;
    
    // Skip size field
    tag = (struct multiboot_tag *)(addr + 8);
    end = addr + total_size;

    vga_print("Parsing Multiboot2 Info...\n");

    while ((unsigned long)tag < end) {
        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
             break;
        }

        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                {
                    struct multiboot_tag_string *tag_cmd = (struct multiboot_tag_string *)tag;
                    vga_print("Cmdline: ");
                    vga_print(tag_cmd->string);
                    vga_print("\n");
                }
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                {
                    struct multiboot_tag_string *tag_name = (struct multiboot_tag_string *)tag;
                    vga_print("Bootloader: ");
                    vga_print(tag_name->string);
                    vga_print("\n");
                }
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                vga_print("Basic Mem Info found.\n");
                break;
            case MULTIBOOT_TAG_TYPE_MMAP:
                {
                    struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
                    struct multiboot_mmap_entry *entry;
                    
                    vga_print("Memory Map:\n");
                    entry = mmap->entries;
                    while ((unsigned long)entry < (unsigned long)mmap + mmap->size) {
                        // Print memory regions (simplified)
                        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                             vga_print("  [RAM] Base: (Hi/Lo) Len: (Hi/Lo)\n");
                             // We lack printf formatting for 64-bit integers on bare metal for now
                        }
                        entry = (struct multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size);
                    }
                }
                break;
        }

        // Align to 8 bytes
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }
}
