// kernel/hal/hal.h - Hardware Abstraction Layer Interface
#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// HAL initialization states
typedef enum {
    HAL_UNINITIALIZED,
    HAL_INITIALIZING,
    HAL_READY,
    HAL_ERROR
} hal_state_t;

// Console operations
typedef struct {
    void (*putc)(char c);
    void (*puts)(const char* str);
    void (*clear)(void);
    int (*getc)(void);  // Non-blocking
    void (*set_color)(uint8_t fg, uint8_t bg);
} hal_console_t;

// Memory operations
typedef struct {
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
    void* (*alloc_aligned)(size_t size, size_t alignment);
    size_t (*get_total)(void);
    size_t (*get_used)(void);
} hal_memory_t;

// Time operations
typedef struct {
    uint64_t (*get_ticks)(void);
    void (*sleep_ms)(uint32_t ms);
    void (*setup_timer)(uint32_t hz);  // For scheduling
} hal_time_t;

// Interrupt operations
typedef struct {
    void (*enable)(void);
    void (*disable)(void);
    void (*register_handler)(int vector, void (*handler)(void));
    void (*acknowledge)(int irq);
} hal_interrupt_t;

// Main HAL structure
typedef struct {
    hal_state_t state;
    const char* name;
    
    // Subsystems
    hal_console_t console;
    hal_memory_t memory;
    hal_time_t time;
    hal_interrupt_t interrupt;
    
    // Initialization
    void (*init)(void);
    void (*shutdown)(void);
} hal_t;

// Global HAL instance
extern hal_t hal;

// Helper macros for kernel code
#define HAL_PUTC(c) hal.console.putc(c)
#define HAL_PUTS(s) hal.console.puts(s)
#define HAL_PRINTF(fmt, ...) hal_printf(fmt, ##__VA_ARGS__)
#define HAL_MALLOC(sz) hal.memory.alloc(sz)
#define HAL_FREE(p) hal.memory.free(p)
#define HAL_TICKS() hal.time.get_ticks()
#define HAL_SLEEP(ms) hal.time.sleep_ms(ms)

// HAL initialization
void hal_early_init(void);
void hal_late_init(void);

// Formatted printing (implemented in hal_stdio.c)
void hal_printf(const char* fmt, ...);
void hal_vprintf(const char* fmt, __builtin_va_list args);
int hal_snprintf(char* buf, size_t size, const char* fmt, ...);
int hal_vsnprintf(char* buf, size_t size, const char* fmt, __builtin_va_list args);

void* hal_memset(void* ptr, int value, size_t num);

#endif // HAL_H
