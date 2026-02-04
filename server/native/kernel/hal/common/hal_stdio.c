// kernel/hal/common/hal_stdio.c - HAL stdio utilities
#include "hal/hal.h"
#include <stdarg.h>

// Helper: format integer
static int format_int(char* buf, int num, int base, int uppercase) {
    static const char* digits_lower = "0123456789abcdef";
    static const char* digits_upper = "0123456789ABCDEF";
    const char* digits = uppercase ? digits_upper : digits_lower;
    
    char temp[32];
    char* p = temp;
    int len = 0;
    
    if (num == 0) {
        *buf++ = '0';
        return 1;
    }
    
    unsigned int unum;
    if (num < 0 && base == 10) {
        *buf++ = '-';
        len++;
        unum = -num;
    } else {
        unum = num;
    }
    
    while (unum > 0) {
        *p++ = digits[unum % base];
        unum /= base;
    }
    
    while (p > temp) {
        *buf++ = *--p;
        len++;
    }
    
    return len;
}

// Simple vprintf
void hal_vprintf(const char* fmt, va_list args) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* str = va_arg(args, char*);
                    while (*str) HAL_PUTC(*str++);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    HAL_PUTC(c);
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char buf[32];
                    int len = format_int(buf, num, 10, 0);
                    for (int i = 0; i < len; i++) {
                        HAL_PUTC(buf[i]);
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buf[32];
                    int len = format_int(buf, num, 16, 0);
                    for (int i = 0; i < len; i++) {
                        HAL_PUTC(buf[i]);
                    }
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buf[32];
                    int len = format_int(buf, num, 16, 1);
                    for (int i = 0; i < len; i++) {
                        HAL_PUTC(buf[i]);
                    }
                    break;
                }
                case 'u': { // Added basic unsigned support
                    unsigned int num = va_arg(args, unsigned int);
                    // format_int handles unsigned if we cast? No, it handles signed.
                    // Let's implement quick unsigned loop or refactor format_int to take unsigned.
                    // For now, hack reusing format_int by passing positive. 
                    // Wait, format_int logic handles negative only if base==10 and num<0.
                    // So passing unsigned as int might be interpreted as negative if MSB set.
                    // Let's just do a quick inline loop for safety.
                    if (num == 0) {
                         HAL_PUTC('0');
                    } else {
                         char temp[32];
                         char* p = temp;
                         while(num > 0) {
                             *p++ = (num % 10) + '0';
                             num /= 10;
                         }
                         while(p > temp) HAL_PUTC(*--p);
                    }
                    break; 
                }
                 case 'l': { // Handle %llu or %ld (minimal hack from previous implementation)
                     if (*(fmt+1) == 'l' && *(fmt+2) == 'u') {
                         fmt += 2;
                         unsigned long long num = va_arg(args, unsigned long long);
                         if (num == 0) HAL_PUTC('0');
                         else {
                             char num_buf[64];
                             char* n = num_buf;
                             while(num > 0) { *n++ = '0' + (num % 10); num /= 10; }
                             while(n > num_buf) HAL_PUTC(*--n);
                         }
                     } else {
                         HAL_PUTC('%'); HAL_PUTC('l');
                     }
                     break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    HAL_PRINTF("0x%x", (unsigned int)(uintptr_t)ptr);
                    break;
                }
                case '%': {
                    HAL_PUTC('%');
                    break;
                }
                default: {
                    HAL_PUTC('%');
                    HAL_PUTC(*fmt);
                    break;
                }
            }
        } else {
            HAL_PUTC(*fmt);
        }
        fmt++;
    }
}

// Simple snprintf
int hal_snprintf(char* buf, size_t size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = hal_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return result;
}

// Simple vsnprintf
int hal_vsnprintf(char* buf, size_t size, const char* fmt, va_list args) {
    if (size == 0) return 0;
    
    char* p = buf;
    size_t remaining = size - 1; // Leave room for null terminator
    
    while (*fmt && remaining > 0) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    while (*str && remaining > 0) {
                        *p++ = *str++;
                        remaining--;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (remaining > 0) {
                        *p++ = c;
                        remaining--;
                    }
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char temp[32];
                    int len = format_int(temp, num, 10, 0);
                    for (int i = 0; i < len && remaining > 0; i++) {
                        *p++ = temp[i];
                        remaining--;
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char temp[32];
                    int len = format_int(temp, num, 16, 0);
                    for (int i = 0; i < len && remaining > 0; i++) {
                        *p++ = temp[i];
                        remaining--;
                    }
                    break;
                }
                 case 'l': {
                     if (*(fmt+1) == 'l' && *(fmt+2) == 'x') { // %llx support
                         fmt += 2;
                         unsigned long long num = va_arg(args, unsigned long long);
                         char temp[64];
                         char* t = temp;
                         if(num==0) *t++='0';
                         else {
                             static const char* hex = "0123456789abcdef";
                             while(num>0) { *t++ = hex[num%16]; num/=16; }
                         }
                         while(t>temp && remaining>0) { *p++ = *--t; remaining--; }
                     } else if (*(fmt+1) == 'l' && *(fmt+2) == 'u') { // %llu
                          fmt += 2;
                          unsigned long long num = va_arg(args, unsigned long long);
                          char temp[64];
                          char* t = temp;
                          if(num==0) *t++='0';
                          else {
                              while(num>0) { *t++ = '0'+(num%10); num/=10; }
                          }
                          while(t>temp && remaining>0) { *p++ = *--t; remaining--; }
                     }
                     break;
                 }
                case '%': {
                    if (remaining > 0) {
                        *p++ = '%';
                        remaining--;
                    }
                    break;
                }
                default: {
                    if (remaining > 0) {
                        *p++ = '%';
                        remaining--;
                    }
                    if (remaining > 0) {
                        *p++ = *fmt;
                        remaining--;
                    }
                    break;
                }
            }
        } else {
            *p++ = *fmt;
            remaining--;
        }
        fmt++;
    }
    
    *p = '\0';
    return p - buf;
}

// Printf wrapper
void hal_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    hal_vprintf(fmt, args);
    va_end(args);
}
