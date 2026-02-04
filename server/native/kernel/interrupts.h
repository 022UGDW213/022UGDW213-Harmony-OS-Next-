#ifndef HARMONY_INTERRUPTS_H
#define HARMONY_INTERRUPTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// IDT Entry Structure (128 bits)
// x86-64 Interrupt Gate Descriptor
typedef struct {
    uint16_t isr_low;       // The lower 16 bits of the ISR's address
    uint16_t kernel_cs;     // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t  ist;           // The IST in the TSS that the CPU will load into RSP; set to 0 for now
    uint8_t  attributes;    // Type and attributes; see the IDT page
    uint16_t isr_mid;       // The middle 16 bits of the ISR's address
    uint32_t isr_high;      // The higher 32 bits of the ISR's address
    uint32_t reserved;      // Set to 0
} __attribute__((packed)) idt_entry_t;

// Pointer to IDT (for lidt instruction)
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

// Interrupt Attributes
#define IDT_TA_INTERRUPT_GATE 0x8E  // 0b10001110 (Present, Ring 0, Interrupt Gate)
#define IDT_TA_TRAP_GATE      0x8F  // 0b10001111 (Present, Ring 0, Trap Gate)
#define IDT_TA_CALL_GATE      0x8C  // 0b10001100 (Present, Ring 0, Call Gate)

// Maximum interrupts
#define IDT_ENTRIES 256

// Interrupt Register State (compatible with assembly push order)
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

// Interrupt Handler Function Pointer
typedef void (*isr_t)(registers_t*);

// API
void interrupts_init(void);
void idt_set_gate(uint8_t vector, void* isr, uint8_t flags);
void register_interrupt_handler(uint8_t vector, isr_t handler);

// ISR Declarations (Assembly)
extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_32();
extern void isr_128(); // Syscall simulation (int 0x80)

// Helper to disable/enable interrupts
static inline void disable_interrupts() {
    // __asm__ volatile("cli"); // Commented out for user-space simulation
}

static inline void enable_interrupts() {
    // __asm__ volatile("sti"); // Commented out for user-space simulation
}

#ifdef __cplusplus
}
#endif

#endif // HARMONY_INTERRUPTS_H
