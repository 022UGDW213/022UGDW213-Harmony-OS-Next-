#ifndef HARMONY_KPRINTF_H
#define HARMONY_KPRINTF_H

// Minimal freestanding printf: writes to both VGA and COM1 serial.
// Supported: %s %c %d %u %x %p %%, plus %llx (printed as two 32-bit
// halves, high first) and %llu (full 64-bit when it fits in 32 bits).
// No libgcc dependency: 64-bit values are handled with shifts only.
void kprintf(const char* fmt, ...);

#endif // HARMONY_KPRINTF_H
