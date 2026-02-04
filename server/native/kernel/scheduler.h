#ifndef HARMONY_SCHEDULER_H
#define HARMONY_SCHEDULER_H

#include "asm/context.h"
#include <stddef.h>
#include <stdint.h>
#include "../security/security.h"

// Forward decl
struct nsproxy;


#ifdef __cplusplus
extern "C" {
#endif

#include "asm/context.h"
#include <stddef.h>
#include <stdint.h>
#include "../security/security.h"
#include "process.h" // Single source of truth

// Forward decl
struct nsproxy;

#ifdef __cplusplus
extern "C" {
#endif

// Scheduler statistics
typedef struct {
    uint64_t total_switches;
    uint64_t total_processes;
    uint64_t running_processes;
    uint64_t quantum_boosts; // Added for qbm
    uint64_t last_update;
} scheduler_stats_int_t; // specific to internal stats? 
// The scheduler.c uses a static struct, but get_stats returns scheduler_stats_t.
// Let's redefine implementation to match.
// For now, I will keep scheduler_stats_t compatible or leave it. 
// scheduler.c defines its own static struct inline! 
// But scheduler_get_stats returns scheduler_stats_t.
// Let's define the public stats struct here.

typedef struct {
    uint64_t total_switches;
    uint64_t total_processes;
    uint64_t running_processes;
} scheduler_stats_t;

// Scheduler initialization

// Scheduler initialization
void scheduler_init(void);
void scheduler_shutdown(void);

// Process managem// Lifecycle
int scheduler_create_process(void (*entry)(void), size_t stack_size);
void scheduler_exit_process(int exit_code);
int scheduler_kill(int pid);
int scheduler_suspend(int pid);
int scheduler_resume(int pid);
process_t* scheduler_get_process(int pid);
process_t* scheduler_get_current_process(void);

// Scheduling
void schedule(void);
void scheduler_yield(void);

// Statistics
scheduler_stats_t scheduler_get_stats(void);
void scheduler_print_stats(void);
void scheduler_print_processes(void);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_SCHEDULER_H
