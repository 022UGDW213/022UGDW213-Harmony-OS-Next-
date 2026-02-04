// kernel/log.c - HAL-enabled logging system
#include "hal/hal.h"
#include "log.h"
#include "hal/config.h" 
#include <stdarg.h>

#define LOG_BUFFER_SIZE 4096
static char log_buffer[LOG_BUFFER_SIZE];
static size_t log_position = 0;

// Log levels as strings
static const char* level_strings[] = {
    "EMERG", "ALERT", "CRIT", "ERR", "WARN", "NOTICE", "INFO", "DEBUG"
};

// Forward declaration
static void log_to_buffer(log_level_t level, const char* fmt, va_list args);

// Circular buffer logging
static void log_to_buffer(log_level_t level, const char* fmt, va_list args) {
    if (log_position >= LOG_BUFFER_SIZE - 256) {
        // Wrap around
        log_position = 0;
    }
    
    char* buf = log_buffer + log_position;
    int remaining = LOG_BUFFER_SIZE - log_position;
    
    // Add timestamp
    int len = 0;
    if (IS_BARE_METAL()) {
        len = hal_snprintf(buf, remaining, "[%08llx] ", HAL_TICKS());
    } else {
        len = hal_snprintf(buf, remaining, "[%llu] ", HAL_TICKS() / 1000); // Convert to ms approx (ticks are ns in hosted)
        // Note: Hosted HAL_TICKS is ns, Bare is ticks. 
        // User script divided by 1,000,000. Let's stick to user script.
    }
    
    // Add log level
    len += hal_snprintf(buf + len, remaining - len, "%-6s: ", 
                       level_strings[level]);
    
    // Add message
    len += hal_vsnprintf(buf + len, remaining - len, fmt, args);
    
    // Add newline
    if (len < remaining - 1) {
        buf[len++] = '\n';
        buf[len] = '\0';
    }
    
    log_position += len;
}

// Public logging API
void kernel_log(log_level_t level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Always log to console
    HAL_PRINTF("%-6s: ", level_strings[level]);
    hal_vprintf(fmt, args);
    HAL_PUTC('\n');
    
    // Store in buffer for dmesg
    log_to_buffer(level, fmt, args);
    
    va_end(args);
}

// Emergency log (always works, even if HAL not fully initialized)
void kernel_log_emergency(const char* msg) {
    // Direct VGA/Serial output for bare-metal emergencies
    if (IS_BARE_METAL()) {
        // Try serial first (more reliable early)
        const char* p = msg;
        while (*p) {
            // Simplified outb for 0x3F8 (COM1)
            // outb(0x3F8, *p) -> outb %al, %dx
            __asm__ volatile("outb %%al, %%dx" : : "a"(*p++), "d"(0x3F8));
        }
        __asm__ volatile("outb %%al, %%dx" : : "a"('\n'), "d"(0x3F8));
        
        // Then VGA if we can
        volatile char* vga = (volatile char*)0xB8000;
        p = msg;
        for (int i = 0; *p && i < 80; i += 2, p++) {
            vga[i] = *p;
            vga[i+1] = 0x0F;
        }
    } else {
        // Hosted: use write() directly
        // write(2, msg, strlen(msg)); we can just use HAL_PUTS or standard printf if panic
        // But user provided code uses write(2..), implying standard unix headers
        // Since we are strictly HAL, let's use HAL_PUTS for safety, OR allow this file to include unistd if HOSTED
        
        #ifdef HARMONY_HOSTED
        // extern ssize_t write(int fildes, const void *buf, size_t nbyte);
        // extern size_t strlen(const char *s);
        // Let's just use HAL_PUTS to be safe and clean
        HAL_PUTS("EMERGENCY: ");
        HAL_PUTS(msg);
        HAL_PUTC('\n');
        #endif
    }
}

// dmesg equivalent
void kernel_dump_log(void) {
    HAL_PRINTF("\n=== Kernel Log (dmesg) ===\n");
    HAL_PRINTF("%s", log_buffer);
    HAL_PRINTF("==========================\n");
}

// Initialize logging subsystem
void log_init(void) {
    kernel_log(LOG_INFO, "Logging subsystem initialized");
    kernel_log(LOG_INFO, "HAL backend: %s", hal.name);
}
