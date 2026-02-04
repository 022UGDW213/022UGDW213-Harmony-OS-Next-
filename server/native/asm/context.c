#include "context.h"
#include <stdio.h>
#include <string.h>

// Initialize a context for a new thread/process
void init_context(context_t* ctx, void (*entry_point)(void), void* stack_top) {
    memset(ctx, 0, sizeof(context_t));
    
    // Align stack to 16 bytes
    uint64_t stack_addr = (uint64_t)stack_top;
    stack_addr &= ~0xF; 
    
    // Push return address (entry point) onto stack
    // Simulating call instruction behavior
    // We subtract 16 to ensure 16-byte alignment of the stack frame
    // and valid RSP (8 mod 16) after retq pops the 8-byte address.
    stack_addr -= 16;
    *(uint64_t*)stack_addr = (uint64_t)entry_point;
    
    // Set up initial register state
    ctx->rsp = stack_addr;
    ctx->rbp = stack_addr; // Frame pointer
    
    // Initialize FPU State (Standard x86 defaults)
    // Offset 0: FCW = 0x037F
    // Offset 24: MXCSR = 0x1F80
    uint16_t* fcw = (uint16_t*)&ctx->fpu_state[0];
    uint32_t* mxcsr = (uint32_t*)&ctx->fpu_state[24];
    *fcw = 0x037F;
    *mxcsr = 0x1F80;
    
    // CRITICAL: context_switch assembly uses offset 0 (rax) to store/restore RIP
    // So we must set it to the entry point for processes switched to via context_switch
    ctx->rax = (uint64_t)entry_point; 
    
    // No need to set rip separately as retq handles it for restore_context
    // But context_switch uses jmp *%rax
    
    printf("📍 Initialized context: entry=%p, stack_top=%p, new_rsp=0x%llx\n", 
           entry_point, stack_top, stack_addr);
}

// Print context for debugging
void print_context(const context_t* ctx) {
    printf("\n📋 CPU Context:\n");
    printf("===============\n");
    printf("rax: 0x%016llx  rbx: 0x%016llx\n", ctx->rax, ctx->rbx);
    printf("rcx: 0x%016llx  rdx: 0x%016llx\n", ctx->rcx, ctx->rdx);
    printf("rsi: 0x%016llx  rdi: 0x%016llx\n", ctx->rsi, ctx->rdi);
    printf("rbp: 0x%016llx  rsp: 0x%016llx\n", ctx->rbp, ctx->rsp);
    printf("r8:  0x%016llx  r9:  0x%016llx\n", ctx->r8, ctx->r9);
    printf("r10: 0x%016llx  r11: 0x%016llx\n", ctx->r10, ctx->r11);
    printf("r12: 0x%016llx  r13: 0x%016llx\n", ctx->r12, ctx->r13);
    printf("r14: 0x%016llx  r15: 0x%016llx\n", ctx->r14, ctx->r15);
    printf("\n");
}
