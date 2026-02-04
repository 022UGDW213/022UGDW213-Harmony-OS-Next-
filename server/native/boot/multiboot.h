#ifndef HARMONY_MULTIBOOT_H
#define HARMONY_MULTIBOOT_H

/* Constants - Visible to C and Assembly */
#define MULTIBOOT2_HEADER_MAGIC      0xe85250d6
#define MULTIBOOT2_BOOTLOADER_MAGIC  0x36d76289
#define MULTIBOOT2_ARCH_I386         0

/* Tag Types */
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16

/* Memory Types */
#define MULTIBOOT_MEMORY_AVAILABLE           1
#define MULTIBOOT_MEMORY_RESERVED            2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE    3
#define MULTIBOOT_MEMORY_NVS                 4
#define MULTIBOOT_MEMORY_BADRAM              5

/* C Declarations - Visible only to C */
#ifndef __ASSEMBLER__

#include <stdint.h>

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry {
        uint64_t addr;
        uint64_t len;
        uint32_t type;
        uint32_t zero;
    } entries[0];
};

/* Function Prototypes */
void multiboot_parse(unsigned long addr);

#endif /* __ASSEMBLER__ */

#endif /* HARMONY_MULTIBOOT_H */
