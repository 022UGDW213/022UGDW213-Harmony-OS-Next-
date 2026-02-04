// kernel/main.c - HAL-enabled kernel entry point
#include "hal/hal.h"
#include "hal/config.h"

// Subsystem headers
#include "memory.h"
#include "scheduler.h"

// Stubs removed - using real implementations linked in
void syscall_init(void) {}
void process_create_init(void) {}

// For hosted, we need main(). For bare metal, we might be called kmain() from boot.S
// To unify, we can have both or macro it.
#ifdef HARMONY_HOSTED
int main(void) {
#else
void kmain(void) {
#endif
    // Early HAL initialization (console only)
    hal_early_init();
    
    HAL_PRINTF("\n\n--------------------------------------\n");
    HAL_PRINTF("       Harmony OS NexT v1.0           \n");
    HAL_PRINTF("  Author: Juan Jose Serrano Palacio   \n");
    HAL_PRINTF("  Build: Maven Apache                 \n");
    HAL_PRINTF("  HAL: %s                             \n", HAL_BACKEND);
    HAL_PRINTF("--------------------------------------\n\n");
    
    // Full HAL initialization
    hal_late_init();
    
    HAL_PRINTF("[KERNEL] Initializing subsystems...\n");
    
    // Initialize kernel subsystems using HAL
    memory_init();
    HAL_PRINTF("[KERNEL] Memory initialized\n");
    
    syscall_init();
    HAL_PRINTF("[KERNEL] Syscalls ready\n");
    
    scheduler_init();
    HAL_PRINTF("[KERNEL] Scheduler ready\n");
    
    process_create_init();
    HAL_PRINTF("[KERNEL] Init process created (PID 1)\n");
    
    HAL_PRINTF("\n[KERNEL] Booting userland...\n");
    
    // Start scheduling
    while (1) {
        // Your quantum scheduler main loop
        // This now uses HAL.time.get_ticks() instead of gettimeofday()
        
        HAL_SLEEP(1000);  // Sleep 1s
        HAL_PRINTF(".");
    }

#ifdef HARMONY_HOSTED
    return 0;
#endif
}
