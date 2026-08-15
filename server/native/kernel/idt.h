#ifndef HARMONY_IDT_H
#define HARMONY_IDT_H

#include <stdint.h>

// 32-bit IDT entry (interrupt gate).
typedef struct {
    uint16_t base_low;    // ISR address, low 16 bits
    uint16_t selector;    // GDT code segment (0x08)
    uint8_t  always0;     // Reserved, must be 0
    uint8_t  flags;       // Type + attributes (0x8E = present ring0 int gate)
    uint16_t base_high;   // ISR address, high 16 bits
} __attribute__((packed)) idt_entry_t;

// IDT pointer for `lidt`.
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

// Register frame saved by kernel/isr.s and passed to handlers.
typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// IRQ handler hook: called for hardware IRQs (vectors 32-47) from isr_handler.
typedef void (*irq_handler_t)(registers_t*);

#define IDT_ENTRIES 48   /* 32 CPU exceptions + 16 remapped PIC IRQs */
#define IRQ_VECTOR_BASE 32

// Stub entry-point table (defined in kernel/isr.s).
extern void (*isr_stub_table[IDT_ENTRIES])(void);

void idt_init(void);
void idt_set_gate(uint8_t vector, uint32_t isr, uint8_t flags);

// Install/query the C hook for a hardware IRQ (0-15).
void irq_install_handler(int irq, irq_handler_t handler);
irq_handler_t irq_get_handler(int irq);

#endif // HARMONY_IDT_H
