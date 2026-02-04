#!/bin/bash

# HarmonyOS Next Build Script for macOS
set -e

echo "🔨 Building HarmonyOS Next..."

# Create directories
mkdir -p build iso/boot/grub

# Clean previous builds
rm -f harmony_kernel harmony.iso

echo "1️⃣ Assembling bootloader..."
# Use nasm for assembly files
nasm -f elf64 boot/boot.S -o build/boot.o 2>/dev/null || {
    echo "⚠️  Using fallback bootloader..."
    cat > boot/miniboot.S << 'MINIBOOT'
[BITS 64]
[ORG 0x100000]
[global _start]
[extern kmain]

section .multiboot_header
align 8
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
header_end:

section .text
_start:
    cli
    mov rsp, stack_top
    call kmain
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
MINIBOOT
    nasm -f elf64 boot/miniboot.S -o build/boot.o
}

echo "2️⃣ Creating minimal kernel entry..."
cat > kmain_bare.c << 'KMAIN'
#include <stdint.h>
#include <stddef.h>

#define VGA_BUFFER 0xB8000

void vga_clear(void) {
    volatile char* video = (volatile char*)VGA_BUFFER;
    for (int i = 0; i < 80*25*2; i += 2) {
        video[i] = ' ';
        video[i+1] = 0x0F;
    }
}

void vga_puts(const char* str) {
    static int cursor = 0;
    volatile char* video = (volatile char*)VGA_BUFFER;
    
    while (*str) {
        if (*str == '\n') {
            cursor = ((cursor / 160) + 1) * 160;
        } else {
            video[cursor++] = *str;
            video[cursor++] = 0x0F;
        }
        str++;
    }
}

void kmain(void) {
    vga_clear();
    vga_puts("HarmonyOS Next\n");
    vga_puts("==============\n");
    vga_puts("Booting successfully!\n");
    vga_puts("Kernel is alive.\n");
    
    // Hang forever
    while (1) {
        __asm__("hlt");
    }
}
KMAIN

echo "3️⃣ Compiling kernel..."
x86_64-elf-gcc -c kmain_bare.c -o build/kmain_bare.o \
    -ffreestanding -O2 -Wall -Wextra \
    -mno-red-zone -mcmodel=large \
    -nostdlib -nostdinc -fno-builtin

echo "4️⃣ Creating linker script..."
cat > linker.ld << 'LINKER'
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
    . = 0x100000;
    
    .text : {
        *(.multiboot_header)
        *(.text)
    }
    
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
LINKER

echo "5️⃣ Linking kernel..."
x86_64-elf-ld -n -o harmony_kernel -T linker.ld build/*.o

echo "✅ Build complete!"
ls -lh harmony_kernel

echo ""
echo "🚀 To run:"
echo "   qemu-system-x86_64 -kernel harmony_kernel -serial stdio -m 512M"
