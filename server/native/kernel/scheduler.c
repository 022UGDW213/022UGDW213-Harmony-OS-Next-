// kernel/scheduler.c - HAL-enabled quantum scheduler
#include "hal/hal.h"
#include "scheduler.h"
#include "process.h"
#include "qbm.h"
#include "log.h"
// #include <string.h> - Removed

#define MAX_PROCESSES 64
#define TIME_SLICE_MS 10  // 10ms time slice

static process_t* process_table[MAX_PROCESSES];
static process_t* current_process = NULL;
static int process_count = 0;

// Quantum scheduling statistics
static struct {
    uint64_t total_switches;
    uint64_t quantum_boosts;
    uint64_t last_update;
} scheduler_stats;

// Initialize scheduler
void scheduler_init(void) {
    kernel_log(LOG_INFO, "Initializing quantum scheduler");
    
    hal_memset(process_table, 0, sizeof(process_table));
    process_count = 0;
    current_process = NULL;
    
    hal_memset(&scheduler_stats, 0, sizeof(scheduler_stats));
    scheduler_stats.last_update = HAL_TICKS();
    
    // Initialize QBM (Quantum Brain Matrix)
    qbm_init();
    
    kernel_log(LOG_INFO, "Scheduler ready (HAL: %s)", hal.name);
}

// Add process to scheduler
int scheduler_add(process_t* proc) {
    if (process_count >= MAX_PROCESSES) {
        kernel_log(LOG_WARN, "Process table full, cannot add PID %d", proc->pid);
        return -1;
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            process_table[i] = proc;
            process_count++;
            
            // Initialize QBM entry for this process
            qbm_add_process(proc->pid);
            
            kernel_log(LOG_INFO, "Added process PID %d (slot %d)", proc->pid, i);
            return 0;
        }
    }
    
    return -1;
}

// Remove process from scheduler
void scheduler_remove(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == pid) {
            // Remove from QBM
            qbm_remove_process(pid);
            
            // If this was the current process, schedule next
            if (current_process == process_table[i]) {
                current_process = NULL;
            }
            
            process_table[i] = NULL;
            process_count--;
            kernel_log(LOG_INFO, "Removed process PID %d", pid);
            return;
        }
    }
    
    kernel_log(LOG_WARN, "Process PID %d not found in scheduler", pid);
}

// Quantum scheduling decision
static process_t* quantum_schedule(void) {
    if (process_count == 0) {
        return NULL;
    }
    
    // Get QBM advice
    qbm_advice_t advice = qbm_get_advice();
    
    // Simple round-robin with quantum boost
    static int last_index = 0;
    int start_index = last_index;
    
    do {
        process_t* proc = process_table[last_index];
        last_index = (last_index + 1) % MAX_PROCESSES;
        
        if (proc && proc->state == PROCESS_READY) {
            // Apply quantum boost if QBM recommends
            if (advice.boost_pid == proc->pid && advice.boost_factor > 0.7) {
                scheduler_stats.quantum_boosts++;
                kernel_log(LOG_DEBUG, "Quantum boost for PID %d (factor: %.2f)", 
                          proc->pid, advice.boost_factor);
            }
            
            return proc;
        }
    } while (last_index != start_index);
    
    return NULL;
}

// Context switch
static void perform_context_switch(process_t* next) {
    if (!next) {
        kernel_log(LOG_WARN, "No process to schedule");
        return;
    }
    
    process_t* prev = current_process;
    
    if (prev == next) {
        return;  // Same process, no switch needed
    }
    
    kernel_log(LOG_DEBUG, "Context switch: %s -> PID %d",
              prev ? "PID " : "(none)", next->pid);
    
    // Update QBM with process behavior
    if (prev) {
        uint64_t runtime = HAL_TICKS() - prev->last_scheduled;
        qbm_update_runtime(prev->pid, runtime);
        prev->state = PROCESS_READY;
    }
    
    // Switch to next process
    next->state = PROCESS_RUNNING;
    next->last_scheduled = HAL_TICKS();
    current_process = next;
    
    scheduler_stats.total_switches++;
    
    // Log scheduling statistics periodically
    static uint64_t last_stat_log = 0;
    uint64_t now = HAL_TICKS();
    if (now - last_stat_log > 5000000) {  // Every 5 seconds
        kernel_log(LOG_INFO, "Scheduler stats: %llu switches, %llu quantum boosts",
                  scheduler_stats.total_switches, scheduler_stats.quantum_boosts);
        last_stat_log = now;
    }
    
    // In hosted mode, we simulate context switch
    // In bare-metal mode, this would call assembly context_switch()
    #ifdef HARMONY_HOSTED
        // Simulated context switch
        kernel_log(LOG_DEBUG, "Simulated context switch to PID %d", next->pid);
    #else
        // Real context switch (when implemented)
        // context_switch(prev ? &prev->context : NULL, &next->context);
    #endif
}

// Scheduler tick (called by timer interrupt)
void scheduler_tick(void) {
    static uint64_t last_schedule = 0;
    uint64_t now = HAL_TICKS();
    
    // Convert ticks to milliseconds (depends on HAL backend)
    #ifdef HARMONY_HOSTED
        uint64_t elapsed_ms = (now - last_schedule) / 1000000;  // ns to ms
    #else
        uint64_t elapsed_ms = (now - last_schedule) / 1000;     // Assuming 1ms ticks
    #endif
    
    if (elapsed_ms >= TIME_SLICE_MS) {
        // Time to schedule
        process_t* next = quantum_schedule();
        if (next) {
            perform_context_switch(next);
        }
        last_schedule = now;
    }
}

// Yield current process
void scheduler_yield(void) {
    if (current_process) {
        current_process->state = PROCESS_READY;
        kernel_log(LOG_DEBUG, "PID %d yielded", current_process->pid);
    }
    
    process_t* next = quantum_schedule();
    if (next) {
        perform_context_switch(next);
    }
}

// Get current process
process_t* scheduler_current(void) {
    return current_process;
}

// Get process by PID
process_t* scheduler_get_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == pid) {
            return process_table[i];
        }
    }
    return NULL;
}

// List all processes
void scheduler_list_processes(void) {
    kernel_log(LOG_INFO, "=== Process List (%d processes) ===", process_count);
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i]) {
            process_t* p = process_table[i];
            const char* state_str = "UNKNOWN";
            switch (p->state) {
                case PROCESS_NEW: state_str = "NEW"; break;
                case PROCESS_READY: state_str = "READY"; break;
                case PROCESS_RUNNING: state_str = "RUNNING"; break;
                case PROCESS_BLOCKED: state_str = "BLOCKED"; break;
                case PROCESS_ZOMBIE: state_str = "ZOMBIE"; break;
            }
            
            kernel_log(LOG_INFO, "PID %4d: %-10s UID:%3d %s", 
                      p->pid, state_str, p->uid, p->name);
        }
    }
}

// Dump scheduler statistics
void scheduler_dump_stats(void) {
    HAL_PRINTF("\n=== Scheduler Statistics ===\n");
    HAL_PRINTF("Total context switches: %llu\n", scheduler_stats.total_switches);
    HAL_PRINTF("Quantum boosts applied: %llu\n", scheduler_stats.quantum_boosts);
    HAL_PRINTF("Active processes: %d/%d\n", process_count, MAX_PROCESSES);
    HAL_PRINTF("Current process: %s\n", 
              current_process ? current_process->name : "(none)");
    HAL_PRINTF("HAL backend: %s\n", hal.name);
    HAL_PRINTF("===========================\n");
}
