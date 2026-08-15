#include "keyboard.h"
#include <stdint.h>

// PS/2 Ports
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD 0x64

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// Scan Code Set 1 (Simplified US layout)
const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void keyboard_init(void) {
    // Determine if PS/2 controller exists (Assume yes for QEMU)
    // In real OS we'd flush buffer, self-test, etc.
}

char keyboard_get_char(void) {
    char c;
    while (!keyboard_get_char_nb(&c)) {
        // Busy wait
    }
    return c;
}

// Non-blocking: read one scancode from the PS/2 data port (0x60) when the
// status register (0x64) bit 0 says data is present. Ignore break codes.
int keyboard_get_char_nb(char* c) {
    if (inb(PS2_STATUS) & 0x1) {
        uint8_t scancode = inb(PS2_DATA);
        if (!(scancode & 0x80) && scancode < sizeof(scancode_map)) {
            char ch = scancode_map[scancode];
            if (ch) {
                *c = ch;
                return 1;
            }
        }
    }
    return 0;
}
