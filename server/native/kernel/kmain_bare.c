#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "../drivers/pic.h"
#include "../boot/multiboot.h"
#include "gdt.h"
#include "idt.h"
#include "kprintf.h"
#include "meminfo.h"
#include "alloc.h"
#include "task.h"
#include "ipc.h"
#include "dev.h"
#include "shell.h"
#include <stdint.h>

// Entry point from assembly (boot/boot.S)
// eax = Multiboot magic, ebx = Multiboot info struct address

// Multiboot 1 bootloader magic (QEMU -kernel direct boot uses the
// Multiboot 1 header from boot/boot.S and passes this value in eax).
#define MB1_BOOTLOADER_MAGIC 0x2BADB002

void kmain(uint32_t magic, uint32_t addr) {
    // 1. Serial first (debug channel from the earliest point)
    serial_init();
    serial_print("[BOOT] HarmonyOS Bare Metal Kernel starting...\n");

    // 2. VGA + banner
    vga_clear();
    vga_set_color(0x0A, 0x00); // Light Green on Black
    vga_print("HarmonyOS Next (Bare Metal Edition)\n");
    vga_print("===================================\n");

    // 3. Multiboot check
    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        vga_print("Multiboot2 Magic Verified\n");
        multiboot_parse(addr); // addr points to a Multiboot2 tag structure
    } else if (magic == MB1_BOOTLOADER_MAGIC) {
        vga_print("Multiboot1 Magic Verified (QEMU -kernel direct boot)\n");
    } else {
        vga_print("!! Invalid Magic Number (Not Multiboot)\n");
    }

    // 4. CPU protections: flat 32-bit GDT, then IDT + PIC remap + ISRs
    serial_print("[INIT] Loading flat 32-bit GDT...\n");
    gdt_init();
    serial_print("[INIT] GDT loaded (code=0x08 data=0x10)\n");

    serial_print("[INIT] Remapping PIC to IRQ vectors 32-47...\n");
    pic_remap(32, 40);       // real outb to 0x20/0xA0/0x21/0xA1
    serial_print("[INIT] Installing IDT (48 gates)...\n");
    idt_init();

    // 5. PIT timer at 100 Hz (real channel-0 mode-2 programming, 0x43/0x40)
    pit_init(100);
    irq_install_handler(0, pit_tick);
    pic_mask_irq(0, 0);      // unmask IRQ0 (timer)
    serial_print("[INIT] PIT timer @ 100 Hz armed, IRQ0 unmasked\n");

    // 6. Real memory detection from the Multiboot 1 info struct
    meminfo_init(addr);

    // 6b. REAL heap allocator: 8 MiB region above the kernel image,
    //     validated against the mmap before a single byte is handed out.
    alloc_init();

    // 7. REAL process manager: the boot context becomes task 0 ("shell")
    //    and two worker tasks are created with kmalloc'd kernel stacks.
    task_init();

    // 7b. The scheduler takes over IRQ0: one round-robin context switch
    //     per PIT tick (scheduler_tick keeps the pit counters alive).
    irq_install_handler(0, scheduler_tick);

    // 8. REAL IPC: ring-buffer mailbox + producer/consumer demo task pair.
    //    Created with interrupts still off so the ring is built atomically.
    ipc_init();

    // 8b. REAL virtual device fabric: vga, serial, keyboard, pit, console.
    dev_init();

    // 9. Enable interrupts: PIT ticks now flow through the IDT + scheduler
    __asm__ volatile ("sti");
    serial_print("[INIT] Interrupts enabled (sti)\n");

    // 8. Interactive shell (PS/2 keyboard + COM1 serial RX)
    vga_print("\nInitial Boot Complete. Interrupts + Shell Active.\n");
    keyboard_init();
    shell_run();

    // shell_run never returns; hang defensively
    while (1) { __asm__ volatile ("hlt"); }
}
