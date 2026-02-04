// kernel/hal/hosted/hal.c - Hosted (Linux/macOS) HAL implementation
#include "../hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

// ========= Console Operations =========
static void hosted_putc(char c) {
    putchar(c);
    fflush(stdout);
}

static void hosted_puts(const char* str) {
    fputs(str, stdout);
    fflush(stdout);
}

static void hosted_clear(void) {
    // ANSI clear screen
    printf("\033[2J\033[H");
}

static int hosted_getc(void) {
    // Non-blocking - return -1 if no input
    return getchar();
}

static void hosted_set_color(uint8_t fg, uint8_t bg) {
    printf("\033[38;5;%dm\033[48;5;%dm", fg, bg);
}

// ========= Memory Operations =========
static void* hosted_alloc(size_t size) {
    return malloc(size);
}

static void hosted_free(void* ptr) {
    free(ptr);
}

static void* hosted_alloc_aligned(size_t size, size_t alignment) {
    void* ptr;
    posix_memalign(&ptr, alignment, size);
    return ptr;
}

static size_t hosted_get_total(void) {
    // Simulated for hosted environment
    return 64 * 1024 * 1024;  // 64MB
}

static size_t hosted_get_used(void) {
    // Track allocations (simplified)
    static size_t used = 0;
    return used;
}

// ========= Time Operations =========
static uint64_t hosted_get_ticks(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void hosted_sleep_ms(uint32_t ms) {
    usleep(ms * 1000);
}

static void hosted_setup_timer(uint32_t hz) {
    // Hosted doesn't need hardware timer setup
    printf("[HAL] Timer would be set to %d Hz (hosted simulation)\n", hz);
}

// ========= Interrupt Operations =========
static void hosted_enable(void) {
    // Nothing to do in hosted
}

static void hosted_disable(void) {
    // Nothing to do in hosted
}

static void hosted_register_handler(int vector, void (*handler)(void)) {
    printf("[HAL] Registered interrupt handler %p for vector %d\n", handler, vector);
}

static void hosted_acknowledge(int irq) {
    // Nothing to do in hosted
}

// ========= HAL Initialization =========
static void hosted_init(void) {
    printf("[HAL] Initializing hosted backend...\n");
}

static void hosted_shutdown(void) {
    printf("[HAL] Shutting down hosted backend...\n");
}

// ========= HAL Instance =========
hal_t hal = {
    .state = HAL_UNINITIALIZED,
    .name = "hosted",
    
    .console = {
        .putc = hosted_putc,
        .puts = hosted_puts,
        .clear = hosted_clear,
        .getc = hosted_getc,
        .set_color = hosted_set_color
    },
    
    .memory = {
        .alloc = hosted_alloc,
        .free = hosted_free,
        .alloc_aligned = hosted_alloc_aligned,
        .get_total = hosted_get_total,
        .get_used = hosted_get_used
    },
    
    .time = {
        .get_ticks = hosted_get_ticks,
        .sleep_ms = hosted_sleep_ms,
        .setup_timer = hosted_setup_timer
    },
    
    .interrupt = {
        .enable = hosted_enable,
        .disable = hosted_disable,
        .register_handler = hosted_register_handler,
        .acknowledge = hosted_acknowledge
    },
    
    .init = hosted_init,
    .shutdown = hosted_shutdown
};
