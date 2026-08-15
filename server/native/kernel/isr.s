/* kernel/isr.s - 32-bit ISR stubs (real CPU exception/IRQ entry points).
 * Each vector gets a real IDT entry; the common stub saves state, calls the
 * C handler isr_handler(registers_t*) and returns with iret. */

.section .text

/* Macro: ISR without a CPU-pushed error code (push a dummy 0). */
.macro ISR_NOERRCODE n
.global isr\n
isr\n:
    cli
    push $0
    push $\n
    jmp isr_common_stub
.endm

/* Macro: ISR whose vector pushes a real error code (8,10-14,17). */
.macro ISR_ERRCODE n
.global isr\n
isr\n:
    cli
    push $\n
    jmp isr_common_stub
.endm

/* Exceptions 0-31 (error-code vectors: 8, 10, 11, 12, 13, 14, 17) */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_ERRCODE   16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

/* Hardware IRQs (PIC remapped to 32-47) */
ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47

/* Common ISR stub: save state, call C, restore, iret. */
isr_common_stub:
    push %gs
    push %fs
    push %es
    push %ds

    pusha

    /* Use the kernel data segment for the handler */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp              /* registers_t* */
    call isr_handler
    add $4, %esp

    popa
    pop %ds
    pop %es
    pop %fs
    pop %gs
    add $8, %esp           /* skip int_no + err_code */
    iret

/* Table of stub addresses so C can fill the IDT without 48 externs. */
.section .data
.global isr_stub_table
isr_stub_table:
    .long isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7
    .long isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15
    .long isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
    .long isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    .long isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39
    .long isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
