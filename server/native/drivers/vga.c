#include "vga.h"

static uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static int vga_col = 0;
static int vga_row = 0;
static uint8_t vga_color = 0x0F; // White on Black

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = (uint16_t)vga_color << 8 | ' ';
        }
    }
    vga_col = 0;
    vga_row = 0;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = bg << 4 | fg;
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        const int index = vga_row * VGA_WIDTH + vga_col;
        VGA_BUFFER[index] = (uint16_t)vga_color << 8 | (uint8_t)c;
        vga_col++;
    }

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    if (vga_row >= VGA_HEIGHT) {
        // Scroll TODO
        vga_row = 0;
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
