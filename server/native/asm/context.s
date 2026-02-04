# Context switching for x86-64 (AT&T syntax for macOS/GCC/Clang)
# Inspired by Linux and xv6 kernel implementations

.global _context_switch
.global _save_context
.global _restore_context

# Context structure layout (must match C struct):
# offset 0:  rax
# offset 8:  rbx
# offset 16: rcx
# offset 24: rdx
# offset 32: rsi
# offset 40: rdi
# offset 48: rbp
# offset 56: rsp
# offset 64: r8-r15 (8 registers * 8 bytes = 64 bytes)

# void context_switch(context_t* old, context_t* new)
# rdi = old context pointer
# rsi = new context pointer
_context_switch:
    # Save callee-saved registers to old context
    movq %rbx, 8(%rdi)
    movq %rbp, 48(%rdi)
    movq %r12, 64(%rdi)
    movq %r13, 72(%rdi)
    movq %r14, 80(%rdi)
    movq %r15, 88(%rdi)
    
    # Save stack pointer
    movq %rsp, 56(%rdi)
    
    # Save return address
    movq (%rsp), %rax
    movq %rax, 0(%rdi)

    # Save FPU/SSE state (Offset 128)
    fxsave 128(%rdi)
    
    # Switch to new context
    # Restore callee-saved registers from new context
    movq 8(%rsi), %rbx
    movq 48(%rsi), %rbp
    movq 64(%rsi), %r12
    movq 72(%rsi), %r13
    movq 80(%rsi), %r14
    movq 88(%rsi), %r15

    # Restore FPU/SSE state (Offset 128)
    fxrstor 128(%rsi)
    
    # Restore stack pointer
    movq 56(%rsi), %rsp
    
    # Jump to saved return address (which is at top of stack)
    ret
    
# void save_context(context_t* ctx)
# rdi = context pointer
_save_context:
    # Save all general-purpose registers
    movq %rax, 0(%rdi)
    movq %rbx, 8(%rdi)
    movq %rcx, 16(%rdi)
    movq %rdx, 24(%rdi)
    movq %rsi, 32(%rdi)
    movq %rdi, 40(%rdi)
    movq %rbp, 48(%rdi)
    movq %rsp, 56(%rdi)
    movq %r8, 64(%rdi)
    movq %r9, 72(%rdi)
    movq %r10, 80(%rdi)
    movq %r11, 88(%rdi)
    movq %r12, 96(%rdi)
    movq %r13, 104(%rdi)
    movq %r14, 112(%rdi)
    movq %r15, 120(%rdi)
    
    ret

# void restore_context(context_t* ctx)
# rdi = context pointer
_restore_context:
    # Restore all general-purpose registers
    movq 0(%rdi), %rax
    movq 8(%rdi), %rbx
    movq 16(%rdi), %rcx
    movq 24(%rdi), %rdx
    movq 32(%rdi), %rsi
    movq 48(%rdi), %rbp
    movq 56(%rdi), %rsp
    movq 64(%rdi), %r8
    movq 72(%rdi), %r9
    movq 80(%rdi), %r10
    movq 88(%rdi), %r11
    movq 96(%rdi), %r12
    movq 104(%rdi), %r13
    movq 112(%rdi), %r14
    movq 120(%rdi), %r15
    movq 40(%rdi), %rdi
    
    ret
