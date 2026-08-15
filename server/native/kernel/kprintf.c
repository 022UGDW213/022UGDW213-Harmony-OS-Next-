// kernel/kprintf.c - minimal freestanding printf (VGA + COM1)
#include "kprintf.h"
#include <stdarg.h>
#include <stdint.h>
#include "../drivers/serial.h"
#include "../drivers/vga.h"

static void kputc(char c) {
    serial_putc(c);
    vga_putc(c);
}

static void kputs(const char* s) {
    while (*s) {
        kputc(*s++);
    }
}

// Write val in the given base into buf (reversed), return its length.
static int utoa(uint32_t val, char* buf, uint32_t base) {
    static const char digits[] = "0123456789abcdef";
    int n = 0;
    do {
        buf[n++] = digits[val % base];
        val /= base;
    } while (val);
    return n;
}

// Emit a number with optional zero-padding to `width` characters.
static void emit_uint32(uint32_t val, uint32_t base, int width) {
    char buf[12];
    int n = utoa(val, buf, base);
    while (n < width) {
        kputc('0');
        width--;
    }
    while (n > 0) {
        kputc(buf[--n]);
    }
}

static void print_uint32(uint32_t val, uint32_t base) {
    emit_uint32(val, base, 0);
}

static void print_int32(int32_t val, int width) {
    if (val < 0) {
        kputc('-');
        val = -val;
        if (width > 0) width--;
    }
    emit_uint32((uint32_t)val, 10, width);
}

// 64-bit hex as high:low (shifts only, no 64-bit division).
static void print_hex64(uint64_t val) {
    uint32_t hi = (uint32_t)(val >> 32);
    uint32_t lo = (uint32_t)(val & 0xFFFFFFFFu);
    if (hi) {
        print_uint32(hi, 16);
        char b[9];
        int n = utoa(lo, b, 16);
        for (int i = n; i < 8; i++) kputc('0');
        while (n > 0) kputc(b[--n]);
    } else {
        print_uint32(lo, 16);
    }
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            kputc(*p);
            continue;
        }
        p++;  // skip '%'

        // Optional zero-padding width, e.g. %08x
        int width = 0;
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }

        int llong = 0;
        while (*p == 'l') {
            llong = 1;
            p++;
        }

        // 64-bit values: hex prints full 64 bits; decimal prints exactly
        // when it fits in 32 bits, otherwise falls back to hex (avoids the
        // 64-bit division that -nostdlib linking cannot resolve).
        if (llong && (*p == 'x' || *p == 'u')) {
            uint64_t v = va_arg(args, uint64_t);
            if (*p == 'x') {
                print_hex64(v);
            } else {
                uint32_t hi = (uint32_t)(v >> 32);
                if (hi) {
                    print_hex64(v);  // >4 GiB: print as hex
                } else {
                    emit_uint32((uint32_t)v, 10, width);
                }
            }
            continue;
        }

        switch (*p) {
            case 's': kputs(va_arg(args, const char*)); break;
            case 'c': kputc((char)va_arg(args, int)); break;
            case 'd': print_int32(va_arg(args, int32_t), width); break;
            case 'u': emit_uint32(va_arg(args, uint32_t), 10, width); break;
            case 'x': emit_uint32(va_arg(args, uint32_t), 16, width); break;
            case 'p': kputc('0'); kputc('x');
                      emit_uint32(va_arg(args, uint32_t), 16, width); break;
            case '%': kputc('%'); break;
            default: kputc('%'); if (*p) kputc(*p); break;
        }
    }

    va_end(args);
}
