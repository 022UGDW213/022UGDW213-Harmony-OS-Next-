// kernel/idt.c - 32-bit Interrupt Descriptor Table + dispatch
#include "idt.h"
#include <stddef.h>
#include "../drivers/pic.h"
#include "../drivers/serial.h"
#include "../drivers/vga.h"
#include "gdt.h"
#include "kprintf.h"

#define IDT_FLAG_INT_GATE 0x8E

static idt_entry_t idt_entries[IDT_ENTRIES];
static idtr_t idtr;
static irq_handler_t irq_handlers[16];

static const char* const exception_names[32] = {
    "Divide Error", "Debug", "NMI", "Breakpoint", "Overflow",
    "Bound Range", "Invalid Opcode", "Device Not Available", "Double Fault",
    "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
    "Stack-Segment Fault", "General Protection Fault", "Page Fault",
    "Reserved", "x87 FPU Error", "Alignment Check", "Machine Check",
    "SIMD FPU Error", "Virtualization", "Control Protection", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved"
};

void idt_set_gate(uint8_t vector, uint32_t isr, uint8_t flags) {
    idt_entries[vector].base_low  = isr & 0xFFFF;
    idt_entries[vector].base_high = (isr >> 16) & 0xFFFF;
    idt_entries[vector].selector  = GDT_KERNEL_CODE;
    idt_entries[vector].always0   = 0;
    idt_entries[vector].flags     = flags;
}

void irq_install_handler(int irq, irq_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

irq_handler_t irq_get_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        return irq_handlers[irq];
    }
    return NULL;
}

// Called from kernel/isr.s with a pointer to the saved register frame.
void isr_handler(registers_t* r) {
    if (r->int_no >= IRQ_VECTOR_BASE && r->int_no < IRQ_VECTOR_BASE + 16) {
        int irq = r->int_no - IRQ_VECTOR_BASE;
        irq_handler_t hook = irq_handlers[irq];
        if (hook) {
            hook(r);
        }
        pic_ack(irq);
    } else if (r->int_no < 32) {
        kprintf("\n[CPU] Exception %u: %s (err=%x) at eip=%x\n",
                r->int_no, exception_names[r->int_no], r->err_code, r->eip);
    } else {
        kprintf("\n[CPU] Unhandled interrupt %u\n", r->int_no);
    }
}

void idt_init(void) {
    // Fill every gate from the real stub table
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)isr_stub_table[i], IDT_FLAG_INT_GATE);
    }

    idtr.limit = sizeof(idt_entries) - 1;
    idtr.base  = (uint32_t)&idt_entries[0];

    __asm__ volatile ( "lidt %0" : : "m"(idtr) : "memory" );
}
