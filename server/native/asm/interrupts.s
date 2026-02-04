# Interrupt Service Routines (ISRs) Stub
# macOS AT&T Syntax

.global _isr_0
.global _isr_1
.global _isr_2
.global _isr_3
.global _isr_32   # Timer
.global _isr_128  # Syscall

.extern _isr_handler

# Macro for ISR without error code
.macro ISR_NOERRCODE num
_isr_\num:
    pushq $0        # Push dummy error code
    pushq $\num     # Push interrupt number
    jmp isr_common_stub
.endm

# Macro for ISR with error code
.macro ISR_ERRCODE num
_isr_\num:
    pushq $\num     # Push interrupt number
    jmp isr_common_stub
.endm

# Define ISRs
ISR_NOERRCODE 0   # Divide by Zero
ISR_NOERRCODE 1   # Debug
ISR_NOERRCODE 2   # NMI
ISR_NOERRCODE 3   # Breakpoint
ISR_NOERRCODE 32  # Timer
ISR_NOERRCODE 128 # Syscall

# Common ISR Stub
isr_common_stub:
    # Save all registers
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Prepare C handler call
    movq %rsp, %rdi     # Pass pointer to stack (registers_t*) as first arg
    
    # Check alignment (stack must be 16-byte aligned for C ABI)
    # This mock assumes we are called in a context where alignment is handled
    # or doesn't matter for the simple printfs we do.
    
    call _isr_handler   # Call C handler

    # Restore registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    # Clean up error code and ISR number
    addq $16, %rsp

    # Return from interrupt
    # iretq  <-- Commented out for user-space simulation (blocks execution flow)
    retq      # Use regular return for testing
