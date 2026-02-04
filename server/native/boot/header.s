/* Multiboot2 Header */
.section .multiboot_header
.align 8

header_start:
    /* Magic number for Multiboot2 */
    .long 0xe85250d6
    /* Architecture: i386 protected mode (0) */
    .long 0
    /* Header length */
    .long header_end - header_start
    /* Checksum */
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    /* Tags end tag (Type 0, Size 8) */
    .word 0
    .word 0
    .long 8
header_end:
