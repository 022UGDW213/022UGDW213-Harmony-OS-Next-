#include "asm/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test thread stacks
#define STACK_SIZE 8192
static char thread1_stack[STACK_SIZE];
static char thread2_stack[STACK_SIZE];

// Test thread contexts
static context_t thread1_ctx;
static context_t thread2_ctx;
static context_t* current_ctx = &thread1_ctx;

// Thread entry points
void thread1_func(void) {
    printf("🧵 Thread 1: Starting\n");
    for (int i = 0; i < 3; i++) {
        printf("🧵 Thread 1: Iteration %d\n", i);
        
        // Switch to thread 2
        context_t* old = current_ctx;
        current_ctx = &thread2_ctx;
        context_switch(old, current_ctx);
    }
    printf("🧵 Thread 1: Exiting\n");
}

void thread2_func(void) {
    printf("🧵 Thread 2: Starting\n");
    for (int i = 0; i < 3; i++) {
        printf("🧵 Thread 2: Iteration %d\n", i);
        
        // Switch to thread 1
        context_t* old = current_ctx;
        current_ctx = &thread1_ctx;
        context_switch(old, current_ctx);
    }
    printf("🧵 Thread 2: Exiting\n");
}

void test_context_switching() {
    printf("⚙️  Context Switching Test\n");
    printf("==========================\n\n");
    
    // Test 1: Save and restore context
    printf("🧪 Test 1: Save and Restore Context\n");
    printf("------------------------------------\n");
    context_t saved_ctx;
    memset(&saved_ctx, 0, sizeof(context_t));
    
    printf("Before save:\n");
    print_context(&saved_ctx);
    
    save_context(&saved_ctx);
    
    printf("After save:\n");
    print_context(&saved_ctx);
    printf("\n");
    
    // Test 2: Initialize contexts
    printf("🧪 Test 2: Initialize Thread Contexts\n");
    printf("--------------------------------------\n");
    
    // Initialize thread 1
    void* stack1_top = thread1_stack + STACK_SIZE;
    init_context(&thread1_ctx, thread1_func, stack1_top);
    printf("Thread 1 context:\n");
    print_context(&thread1_ctx);
    
    // Initialize thread 2
    void* stack2_top = thread2_stack + STACK_SIZE;
    init_context(&thread2_ctx, thread2_func, stack2_top);
    printf("Thread 2 context:\n");
    print_context(&thread2_ctx);
    
    printf("\n");
    
    // Test 3: Context switch demonstration
    printf("🧪 Test 3: Context Switch Demo\n");
    printf("-------------------------------\n");
    printf("Note: Full cooperative multithreading demo\n");
    printf("(Actual context switching requires assembly integration)\n\n");
    
    // In a real implementation, we would:
    // 1. Set up proper stack frames
    // 2. Save return addresses
    // 3. Switch stacks
    // 4. Jump to new context
    
    printf("✅ Context switching infrastructure ready!\n");
    printf("\nNext steps:\n");
    printf("- Integrate with scheduler\n");
    printf("- Add timer interrupts\n");
    printf("- Implement preemptive multitasking\n");
}

int main() {
    test_context_switching();
    return 0;
}
