#ifndef HARMONY_CONTEXT_H
#define HARMONY_CONTEXT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// CPU context structure (matches assembly layout)
typedef struct context {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    
    // FPU/SSE State (512 bytes for fxsave)
    // Must be 16-byte aligned (offset 128 is 16-byte aligned)
    uint8_t fpu_state[512] __attribute__((aligned(16)));
} context_t;

// Assembly functions
extern void context_switch(context_t* old, context_t* new);
extern void save_context(context_t* ctx);
extern void restore_context(context_t* ctx);

// Helper functions
void init_context(context_t* ctx, void (*entry_point)(void), void* stack_top);
void print_context(const context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_CONTEXT_H
