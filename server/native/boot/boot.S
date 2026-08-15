/* boot/boot.S - Multiboot Entry Point */
#define ASM_FILE 1
#include "multiboot.h"

/* Multiboot 1 Header (Legacy - For QEMU -kernel support) */
#define MB1_MAGIC    0x1BADB002
#define MB1_FLAGS    0x03  /* Align modules + Mem Info */
#define MB1_CHECKSUM -(MB1_MAGIC + MB1_FLAGS)

.section .multiboot_header, "a"   /* allocatable: stays first in the LOAD segment (QEMU scans the first 8KiB of the file) */
.align 4
mb1_header_start:
    .long MB1_MAGIC
    .long MB1_FLAGS
    .long MB1_CHECKSUM

    /* AOUT Kludge (Optional, but safe to omit if ELF) */
    
.align 8
/* Multiboot 2 Header (Disabled for QEMU -kernel Direct Boot) */
/*
header_start:
    .long MB_MAGIC
    .long MB_ARCH
    .long header_end - header_start
    // Checksum: -(magic + arch + length) 
    .long -(MB_MAGIC + MB_ARCH + (header_end - header_start))

    // Tags: End Tag 
    .word 0
    .word 0
    .long 8
header_end:
*/

.section .text
.code32
.global _start
.extern kmain

_start:
    /* Interrupts disabled by bootloader, but good practice */
    cli

    /* Setup Stack */
    mov $stack_top, %esp
    
    /* Align stack to 16 bytes (System V ABI) */
    and $-16, %esp

    /* Push Multiboot Magic and Addr as arguments to kmain */
    /* void kmain(uint32_t magic, uint32_t addr) */
    /* C calling convention (32-bit): push args right-to-left */
    push %ebx /* Multiboot Info Struct Address */
    push %eax /* Multiboot Magic Value */

    /* Reset EFLAGS */
    push $0
    popf

    /* Call Kernel Main */
    call kmain

    /* Hang */
    cli
1:  hlt
    jmp 1b

.section .bss
.align 16
stack_bottom:
.skip 16384 /* 16KB Stack */
stack_top:
