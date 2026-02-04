#include "kernel/interrupts.h"
#include <stdio.h>
#include <string.h>

// Global IDT
__attribute__((aligned(0x10))) 
static idt_entry_t idt[IDT_ENTRIES];
static idtr_t idtr;

// Handler Table
static isr_t interrupt_handlers[IDT_ENTRIES];

// Default handler
void isr_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        printf("⚠️  Unhandled interrupt: %llu (Error Code: %llu)\n", regs->int_no, regs->err_code);
    }
}

// Set an entry in the IDT
void idt_set_gate(uint8_t vector, void* isr, uint8_t flags) {
    idtr_t* lidt = &idtr; // Dummy usage for analysis
    (void)lidt;

    uint64_t addr = (uint64_t)isr;
    
    idt[vector].isr_low = addr & 0xFFFF;
    idt[vector].kernel_cs = 0x08; // Kernel code segment offset
    idt[vector].ist = 0;
    idt[vector].attributes = flags;
    idt[vector].isr_mid = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

// Install IDT
void interrupts_init(void) {
    memset(&idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);
    memset(&interrupt_handlers, 0, sizeof(isr_t) * IDT_ENTRIES);

    idtr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idtr.base = (uint64_t)&idt;

    // Load IDT (Assembly wrapper usually calls 'lidt', here we simulated structure setup)
    // __asm__ volatile("lidt %0" : : "m"(idtr));
    
    // Set gates (example)
    idt_set_gate(0, isr_0, IDT_TA_INTERRUPT_GATE);
    idt_set_gate(1, isr_1, IDT_TA_INTERRUPT_GATE);
    idt_set_gate(2, isr_2, IDT_TA_INTERRUPT_GATE);
    idt_set_gate(3, isr_3, IDT_TA_INTERRUPT_GATE);
    idt_set_gate(32, isr_32, IDT_TA_INTERRUPT_GATE);
    idt_set_gate(128, isr_128, IDT_TA_INTERRUPT_GATE);

    printf("⚙️  IDT Initialized at %p (Limit: %d)\n", &idt, idtr.limit);
}

// Register a high-level handler
void register_interrupt_handler(uint8_t vector, isr_t handler) {
    interrupt_handlers[vector] = handler;
}
