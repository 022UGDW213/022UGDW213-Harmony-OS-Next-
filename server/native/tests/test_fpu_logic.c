#include <stdio.h>
#include <stdint.h>
#include "../kernel/scheduler.h"
#include "../kernel/syscall.h"

// Volatile to prevent optimization
volatile float val = 0.0f;

void fpu_process_a() {
    float a = 1.5f;
    for (int i = 0; i < 5; i++) {
        a += 1.1f;
        printf("[PROC A] Float: %.2f (Expected: %.2f)\n", a, 1.5f + (1.1f * (i+1)));
        scheduler_yield();
    }
    printf("✅ PROC A Complete\n");
    scheduler_exit_process(0);
}

void fpu_process_b() {
    double b = 100.0;
    for (int i = 0; i < 5; i++) {
        b /= 2.0;
        printf("[PROC B] Double: %.2f (Expected: %.2f)\n", b, 100.0 / (1 << (i+1)));
        scheduler_yield();
    }
    printf("✅ PROC B Complete\n");
    scheduler_exit_process(0);
}

int main() {
    printf("🧪 Starting FPU Context Switch Test...\n");
    
    // Manual setup since we are running as a standalone test linked partly to kernel
    // But wait, to properly test context switch we need the kernel scheduler running.
    // This file should be linked INTO the kernel as a test module or we extend main.c
    
    printf("Please run this via the main kernel shell or init process.\n");
    return 0;
}
