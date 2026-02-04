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
    
    /DISCARD/ : { *(.note.*) *(.comment) }
}
