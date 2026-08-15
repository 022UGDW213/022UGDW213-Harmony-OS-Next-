#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/keyboard.h"
#include "../boot/multiboot.h"

// Entry point from assembly
// ebx contains multiboot header address

// Multiboot 1 bootloader magic (QEMU -kernel direct boot uses the
// Multiboot 1 header from boot/boot.S and passes this value in eax).
#define MB1_BOOTLOADER_MAGIC 0x2BADB002

void kmain(uint32_t magic, uint32_t addr) {
    // 1. Initialize Drivers
    vga_clear();
    vga_set_color(0x0A, 0x00); // Light Green on Black
    vga_print("HarmonyOS Next (Bare Metal Edition)\n");
    vga_print("===================================\n");
    
    serial_init();
    serial_print("[BOOT] HarmonyOS Bare Metal Kernel starting...\n");
    
    // 2. Multiboot Check
    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        vga_print("Multiboot2 Magic Verified\n");
        multiboot_parse(addr); // addr points to a Multiboot2 tag structure
    } else if (magic == MB1_BOOTLOADER_MAGIC) {
        vga_print("Multiboot1 Magic Verified (QEMU -kernel direct boot)\n");
        // addr points to a Multiboot1 info struct (MB2 layout parser N/A)
    } else {
        vga_print("!! Invalid Magic Number (Not Multiboot)\n");
    }
    
    // 3. Interactive Loop
    vga_print("\nInitial Boot Complete. Keyboard Active.\n");
    vga_print("Type something: ");
    
    keyboard_init();
    
    while(1) {
        char c = keyboard_get_char();
        vga_putc(c);
        
        // Echo to serial for debugging
        if (c == '\n') serial_print("\r\n");
        else {
             char tmp[2] = {c, 0};
             serial_print(tmp);
        }
    }
}
