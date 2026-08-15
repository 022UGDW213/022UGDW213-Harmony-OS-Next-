#ifndef HARMONY_DEV_H
#define HARMONY_DEV_H

#include <stdint.h>

// REAL virtual device fabric: a fixed device table (vga, serial, keyboard,
// pit, console) whose open/read/write slots are real function pointers that
// call the actual drivers. The `devices` shell command lists the table.

#define DEV_MAX     16
#define DEV_NAME_MAX 16

#define DEV_TYPE_CHAR  1
#define DEV_TYPE_TIMER 2
#define DEV_TYPE_NULL  0

typedef struct device {
    char     name[DEV_NAME_MAX];
    uint32_t type;
    int  (*open)(struct device* d);                       // optional
    int  (*read)(struct device* d, void* buf, uint32_t len);  // optional
    int  (*write)(struct device* d, const void* buf, uint32_t len); // optional
    void* priv;
} device_t;

// Register a device; returns its index or -1 when the table is full.
int dev_register(const char* name, uint32_t type,
                 int (*open)(device_t*),
                 int (*read)(device_t*, void*, uint32_t),
                 int (*write)(device_t*, const void*, uint32_t),
                 void* priv);

// Boot the fabric: register vga, serial, keyboard, pit, console with their
// real driver-backed operations.
void dev_init(void);

// Introspection for the `devices` shell command.
uint32_t dev_count(void);
const device_t* dev_get(uint32_t i);   // NULL past the end

#endif // HARMONY_DEV_H
