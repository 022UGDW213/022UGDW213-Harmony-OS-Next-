// drivers/pic.c - 8259 Programmable Interrupt Controller, real port I/O
#include "pic.h"

// Master PIC ports
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
// Slave PIC ports
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01
#define PIC_EOI    0x20

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void pic_remap(uint8_t master_offset, uint8_t slave_offset) {
    // ICW1: initialization command word (cascade mode, expect ICW4)
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    // ICW2: vector offsets
    outb(PIC1_DATA, master_offset);
    outb(PIC2_DATA, slave_offset);
    // ICW3: cascade wiring (slave on IRQ2 of master; slave cascade id 2)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    // ICW4: 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    // Mask all IRQs on both PICs (kernel unmasks what it needs)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_ack(int irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}

void pic_mask_irq(int irq, uint8_t masked) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t bit = 1u << (irq & 7);
    uint8_t mask = inb(port);
    if (masked) {
        mask |= bit;
    } else {
        mask &= (uint8_t)~bit;
    }
    outb(port, mask);
}
