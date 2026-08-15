// kernel/hal/bare/hal.c - Bare Metal HAL implementation
#include "../hal.h"
#include "../../../drivers/vga.h"
#include "../../../drivers/serial.h"
#include "../../../drivers/pit.h"
#include <stdint.h>

// Real PIT driver provides pit_get_ticks() and pit_init() (see drivers/pit.c)


// ========= Console Operations =========
static void bare_putc(char c) {
    // Try serial first (for debugging), then VGA
    serial_putc(c);
    vga_putc(c);
}

static void bare_puts(const char* str) {
    while (*str) {
        bare_putc(*str++);
    }
}

static void bare_clear(void) {
    vga_clear();
}

static int bare_getc(void) {
    // TODO: Implement keyboard driver
    return -1;  // No input available
}

static void bare_set_color(uint8_t fg, uint8_t bg) {
    vga_set_color(fg, bg);
}

// ========= Memory Operations =========
// These will be implemented once your physical memory manager is ported
static void* bare_alloc(size_t size) {
    // TODO: Hook into your physical allocator
    // For now, just panic
    bare_puts("[HAL] Memory allocator not initialized!\n");
    while(1);  // Hang
    return NULL;
}

static void bare_free(void* ptr) {
    // TODO: Implement
}

static void* bare_alloc_aligned(size_t size, size_t alignment) {
    // TODO: Implement
    return NULL;
}

static size_t bare_get_total(void) {
    // Will be set during memory detection
    // extern size_t total_memory;
    // return total_memory;
    return 128 * 1024 * 1024; // Stub 128MB
}

static size_t bare_get_used(void) {
    // extern size_t used_memory;
    // return used_memory;
    return 0; // Stub
}

// ========= Time Operations =========
static uint64_t bare_get_ticks(void) {
    return pit_get_ticks();
}

static void bare_sleep_ms(uint32_t ms) {
    // volatile loop for now since PIT might not be ready
    for(int i=0; i<ms*10000; i++) __asm__("pause");
}

static void bare_setup_timer(uint32_t hz) {
    pit_init(hz);
}

// ========= Interrupt Operations =========
static void bare_enable(void) {
    __asm__("sti");
}

static void bare_disable(void) {
    __asm__("cli");
}

static void bare_register_handler(int vector, void (*handler)(void)) {
    // TODO: Implement IDT registration
}

static void bare_acknowledge(int irq) {
    // TODO: PIC/APIC EOI
}

// ========= HAL Initialization =========
static void bare_init(void) {
    bare_puts("[HAL] Initializing bare metal backend...\n");
    
    // Initialize drivers
    // vga_init(); // VGA doesn't have explicit init, just clear
    hal.console.clear();
    serial_init(); // Updated to match existing serial.c
    bare_setup_timer(1000);  // 1ms ticks
    
    bare_puts("[HAL] Bare metal backend ready\n");
}

static void bare_shutdown(void) {
    bare_puts("[HAL] Shutting down...\n");
    while(1);  // Can't really shutdown on bare metal
}

// ========= HAL Instance =========
hal_t hal = {
    .state = HAL_UNINITIALIZED,
    .name = "bare-metal",
    
    .console = {
        .putc = bare_putc,
        .puts = bare_puts,
        .clear = bare_clear,
        .getc = bare_getc,
        .set_color = bare_set_color
    },
    
    .memory = {
        .alloc = bare_alloc,
        .free = bare_free,
        .alloc_aligned = bare_alloc_aligned,
        .get_total = bare_get_total,
        .get_used = bare_get_used
    },
    
    .time = {
        .get_ticks = bare_get_ticks,
        .sleep_ms = bare_sleep_ms,
        .setup_timer = bare_setup_timer
    },
    
    .interrupt = {
        .enable = bare_enable,
        .disable = bare_disable,
        .register_handler = bare_register_handler,
        .acknowledge = bare_acknowledge
    },
    
    .init = bare_init,
    .shutdown = bare_shutdown
};
