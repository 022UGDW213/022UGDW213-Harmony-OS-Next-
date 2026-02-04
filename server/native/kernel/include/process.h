#ifndef HARMONY_PROCESS_H
#define HARMONY_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include "hal/hal.h"

// Process states
typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_ZOMBIE
} process_state_t;

// Process Control Block
typedef struct process {
    int pid;
    int uid;
    char name[32];
    process_state_t state;
    
    // Scheduling info
    uint64_t last_scheduled;
    uint64_t total_runtime;
    int priority;
    
    // Memory
    void* stack;
    size_t stack_size;
    
    // Execution Context (added for scheduler.c compatibility)
    #include "asm/context.h"
    context_t context;

    struct process* next;
} process_t;

#endif
