#ifndef LOG_H
#define LOG_H

typedef enum {
    LOG_EMERG = 0,   // System is unusable
    LOG_ALERT,       // Action must be taken immediately
    LOG_CRIT,        // Critical conditions
    LOG_ERR,         // Error conditions
    LOG_WARN,        // Warning conditions
    LOG_NOTICE,      // Normal but significant condition
    LOG_INFO,        // Informational messages
    LOG_DEBUG        // Debug-level messages
} log_level_t;

// Main logging function
void kernel_log(log_level_t level, const char* fmt, ...);

// Emergency log (works even if HAL not ready)
void kernel_log_emergency(const char* msg);

// Initialize logging subsystem
void log_init(void);

// Dump kernel log buffer
void kernel_dump_log(void);

// Helper macros
#define LOG_EMERG(...)   kernel_log(LOG_EMERG, __VA_ARGS__)
#define LOG_ALERT(...)   kernel_log(LOG_ALERT, __VA_ARGS__)
#define LOG_CRIT(...)    kernel_log(LOG_CRIT, __VA_ARGS__)
#define LOG_ERR(...)     kernel_log(LOG_ERR, __VA_ARGS__)
#define LOG_WARN(...)    kernel_log(LOG_WARN, __VA_ARGS__)
#define LOG_NOTICE(...)  kernel_log(LOG_NOTICE, __VA_ARGS__)
#define LOG_INFO(...)    kernel_log(LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(...)   kernel_log(LOG_DEBUG, __VA_ARGS__)

#endif // LOG_H
