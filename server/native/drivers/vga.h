#ifndef HARMONY_VGA_H
#define HARMONY_VGA_H

#include <stdint.h>

void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putc(char c);
void vga_print(const char* str);
void vga_set_color(uint8_t fg, uint8_t bg);

#endif
