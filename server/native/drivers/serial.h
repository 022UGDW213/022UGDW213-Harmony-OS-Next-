#ifndef HARMONY_SERIAL_H
#define HARMONY_SERIAL_H

#include <stdint.h>

void serial_init(void);
void serial_putc(char c);
void serial_print(const char* str);

// Non-blocking read: returns 1 and stores the char in *c when a byte
// is waiting on COM1 RX (line status register 0x3FD bit 0), else 0.
int serial_getc(char* c);

#endif
