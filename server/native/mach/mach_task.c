#include "mach/task.h"
#include "kernel/scheduler.h"
#include <stdio.h>
#include <stdlib.h>

// For simulation, we map task_t (port) directly to PID for now
// In a real microkernel, task_t is a port that maps to a Task Object which contains the PID.

// Helper mapping (Simple identity for simulation or map via port table)
// We will use a simple registry: Port ID -> PID.
// Reusing port_table logic would be ideal if exposed, but for now we mock it.

#define MAX_TASKS 64
static struct {
    mach_port_t port;
    int pid;
    bool active;
} task_table[MAX_TASKS];

static int task_count = 0;

// Internal helper to find task slot
static int find_task_slot(task_t task) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_table[i].active && task_table[i].port == task) {
            return i;
        }
    }
    return -1;
}

task_t mach_task_self(void) {
    // In simulation, we are usually "kernel", but if we track current thread...
    // For now return a dummy self
    return 1;
}

// Create a new task (wrapper around process creation)
// Note: In Mach, task_create creates an empty task (no threads).
// We will spawn a dummy process to represent the task.
static void dummy_task_entry(void) {
    while (1) {
        scheduler_yield();
    }
}

task_t task_create(void) {
    // 1. Allocate a port
    mach_port_t port = mach_port_allocate();
    if (port == MACH_PORT_NULL) return MACH_TASK_NULL;
    
    // 2. Create the underlying process
    int pid = scheduler_create_process(dummy_task_entry, 0); // 0 = default stack
    if (pid < 0) {
        mach_port_deallocate(port);
        return MACH_TASK_NULL;
    }
    
    // 3. Register
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_table[i].active) {
            task_table[i].port = port;
            task_table[i].pid = pid;
            task_table[i].active = true;
            task_count++;
            
            printf("   [Mach] Task Created: Port %d -> PID %d\n", port, pid);
            return port;
        }
    }
    
    // Table full
    mach_port_deallocate(port);
    scheduler_exit_process(pid); // How to kill specific pid? We need scheduler_kill(pid)
    return MACH_TASK_NULL;
}

void task_terminate(task_t task) {
    int idx = find_task_slot(task);
    if (idx < 0) return;
    
    int pid = task_table[idx].pid;
    printf("   [Mach] Terminating Task Port %d (PID %d)\n", task, pid);
    
    // Call scheduler to kill process (We need to expose this in scheduler.h)
    // For now assuming we can add it.
    scheduler_kill(pid);
    
    mach_port_deallocate(task);
    task_table[idx].active = false;
    task_count--;
}

void task_suspend(task_t task) {
    int idx = find_task_slot(task);
    if (idx < 0) return;
    
    int pid = task_table[idx].pid;
    printf("   [Mach] Suspending Task Port %d (PID %d)\n", task, pid);
    scheduler_suspend(pid);
}

void task_resume(task_t task) {
    int idx = find_task_slot(task);
    if (idx < 0) return;
    
    int pid = task_table[idx].pid;
    printf("   [Mach] Resuming Task Port %d (PID %d)\n", task, pid);
    scheduler_resume(pid);
}

int task_pid(task_t task) {
    int idx = find_task_slot(task);
    if (idx < 0) return -1;
    return task_table[idx].pid;
}

// Reverse lookup for BSD Shim
task_t task_from_pid(int pid) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_table[i].active && task_table[i].pid == pid) {
            return task_table[i].port;
        }
    }
    return MACH_TASK_NULL;
}
