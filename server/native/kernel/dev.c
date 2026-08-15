// kernel/dev.c - REAL virtual device fabric.
//
// A fixed device table whose per-device open/read/write slots are real
// function pointers into the actual drivers: VGA text writes, COM1 UART
// read/write, PS/2 keyboard read, PIT tick read and a console that mirrors
// to VGA + serial. The `devices` shell command prints the live table.
#include "dev.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "kprintf.h"
#include <stddef.h>
#include <stdint.h>

static device_t devices[DEV_MAX];
static uint32_t device_count = 0;

static void set_name(char* dst, uint32_t cap, const char* src) {
    uint32_t i = 0;
    while (src[i] && i < cap - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// ── real per-device operations (straight into the drivers) ─────────────────
static int dev_vga_open(device_t* d) {
    (void)d;
    vga_clear();
    return 0;
}

static int dev_vga_write(device_t* d, const void* buf, uint32_t len) {
    (void)d;
    const char* p = (const char*)buf;
    for (uint32_t i = 0; i < len; i++) {
        vga_putc(p[i]);
    }
    return (int)len;
}

static int dev_serial_read(device_t* d, void* buf, uint32_t len) {
    (void)d;
    if (len == 0) {
        return 0;
    }
    char c;
    if (serial_getc(&c)) {
        ((char*)buf)[0] = c;
        return 1;
    }
    return 0;
}

static int dev_serial_write(device_t* d, const void* buf, uint32_t len) {
    (void)d;
    const char* p = (const char*)buf;
    for (uint32_t i = 0; i < len; i++) {
        serial_putc(p[i]);
    }
    return (int)len;
}

static int dev_keyboard_open(device_t* d) {
    (void)d;
    keyboard_init();
    return 0;
}

static int dev_keyboard_read(device_t* d, void* buf, uint32_t len) {
    (void)d;
    if (len == 0) {
        return 0;
    }
    char c;
    if (keyboard_get_char_nb(&c)) {
        ((char*)buf)[0] = c;
        return 1;
    }
    return 0;
}

// PIT as a device: a read returns the live tick counter (low 32 bits,
// little-endian) and reports 4 bytes consumed.
static int dev_pit_read(device_t* d, void* buf, uint32_t len) {
    (void)d;
    if (len < 4) {
        return 0;
    }
    uint32_t ticks = (uint32_t)pit_get_ticks();
    uint8_t* p = (uint8_t*)buf;
    p[0] = (uint8_t)(ticks & 0xFF);
    p[1] = (uint8_t)((ticks >> 8) & 0xFF);
    p[2] = (uint8_t)((ticks >> 16) & 0xFF);
    p[3] = (uint8_t)((ticks >> 24) & 0xFF);
    return 4;
}

static int dev_console_read(device_t* d, void* buf, uint32_t len) {
    (void)d;
    if (len == 0) {
        return 0;
    }
    char c;
    if (serial_getc(&c)) {
        ((char*)buf)[0] = c;
        return 1;
    }
    if (keyboard_get_char_nb(&c)) {
        ((char*)buf)[0] = c;
        return 1;
    }
    return 0;
}

static int dev_console_write(device_t* d, const void* buf, uint32_t len) {
    (void)d;
    const char* p = (const char*)buf;
    for (uint32_t i = 0; i < len; i++) {
        serial_putc(p[i]);
        vga_putc(p[i]);
    }
    return (int)len;
}

int dev_register(const char* name, uint32_t type,
                 int (*open)(device_t*),
                 int (*read)(device_t*, void*, uint32_t),
                 int (*write)(device_t*, const void*, uint32_t),
                 void* priv) {
    if (device_count >= DEV_MAX) {
        return -1;
    }
    device_t* d = &devices[device_count];
    set_name(d->name, DEV_NAME_MAX, name);
    d->type = type;
    d->open = open;
    d->read = read;
    d->write = write;
    d->priv = priv;
    return (int)device_count++;
}

void dev_init(void) {
    dev_register("vga",      DEV_TYPE_CHAR,  dev_vga_open,
                 NULL, dev_vga_write, NULL);
    dev_register("serial",   DEV_TYPE_CHAR,  NULL,
                 dev_serial_read, dev_serial_write, NULL);
    dev_register("keyboard", DEV_TYPE_CHAR,  dev_keyboard_open,
                 dev_keyboard_read, NULL, NULL);
    dev_register("pit",      DEV_TYPE_TIMER, NULL,
                 dev_pit_read, NULL, NULL);
    dev_register("console",  DEV_TYPE_CHAR,  NULL,
                 dev_console_read, dev_console_write, NULL);
    kprintf("[DEV] fabric ready: %u devices registered\n", dev_count());
}

uint32_t dev_count(void) { return device_count; }

const device_t* dev_get(uint32_t i) {
    return (i < device_count) ? &devices[i] : NULL;
}
