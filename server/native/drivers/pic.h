#ifndef HARMONY_PIC_H
#define HARMONY_PIC_H

#include <stdint.h>

// Remap the 8259 PIC pair so hardware IRQs live at IDT vectors 32-47
// (instead of the default 0-15 which collide with CPU exceptions).
void pic_remap(uint8_t master_offset, uint8_t slave_offset);

// Send End-Of-Interrupt to the master (and slave when needed).
void pic_ack(int irq);

// Mask individual IRQ lines (1 = masked/disabled).
void pic_mask_irq(int irq, uint8_t masked);

#endif // HARMONY_PIC_H
