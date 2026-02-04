// test_hal.c
#include "kernel/hal/hal.h"
#include <stdio.h>

// Mock memory init/scheduler init for linking
void memory_init(void) {}
void scheduler_init(void) {}
void syscall_init(void) {}
void process_create_init(void) {}

int main() {
    printf("Starting HAL Test...\n");
    hal_early_init();
    
    HAL_PRINTF("HAL Verified: %s backend active\n", hal.name);
    HAL_PRINTF("HAL_PRINTF Test: Number=%d Hex=0x%x String='%s'\n", 123, 0xABC, "Success");
    
    hal_late_init();
    
    // Allocate memory check
    void* ptr = HAL_MALLOC(100);
    if (ptr) {
        HAL_PRINTF("Memory Alloc: Success (%p)\n", ptr);
        HAL_FREE(ptr);
    } else {
        HAL_PRINTF("Memory Alloc: FAILED\n");
    }
    
    return 0;
}
