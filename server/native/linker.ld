/* Linker Script for HarmonyOS Next (Bare Metal i386) */
ENTRY(_start)

SECTIONS {
    /* Kernel loaded at 1MB */
    . = 1M;

    /* Multiboot header must be within first 8KB */
    .boot :
    {
        KEEP(*(.multiboot_header))
    }

    .text :
    {
        *(.text)
        *(.text.*)
    }

    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data :
    {
        *(.data)
        *(.data.*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Kernel heap region: 8 MiB of free physical RAM directly above the
     * kernel image. Pure symbol math (no output is emitted into the ELF);
     * alloc_init() validates the range against the real Multiboot mmap
     * before carving blocks out of it. */
    . = ALIGN(16);
    __kernel_end = .;
    __heap_start = __kernel_end;
    __heap_end = __heap_start + 8M;

    /DISCARD/ : { *(.note.*) *(.comment) }
}
