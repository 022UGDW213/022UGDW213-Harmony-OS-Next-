#include "bsd/signal.h"
#include "mach/task.h"
#include "mach/message.h"
#include "kernel/scheduler.h"
#include <stdio.h>

// BSD Shim Layer
// Translates POSIX signals into Mach IPC messages or Task operations.

// Internal: Lookup task by PID (Simulated: In real Mach, task_for_pid requires special rights)
// We need a reverse lookup or just access to our global task table? 
// mach_task.c has the table but it's static. 
// We should expose a helper `task_from_pid(pid)` in mach/task.h or mach/mach_task.c

extern task_t task_from_pid(int pid); // We need to add this to mach_task.c

int bsd_kill(int pid, int sig) {
    printf("   [BSD] kill(%d, %d)\n", pid, sig);
    
    // 1. Resolve PID to Mach Task
    task_t target_task = task_from_pid(pid);
    if (target_task == MACH_TASK_NULL) {
        printf("   [BSD] ESRCH: Non-existent process %d\n", pid);
        return -1; // ESRCH
    }
    
    // 2. Translate Signal to Mach Operation
    // In XNU, this sends an exception message. Here we simplify:
    // Some signals default to task control logic directly here (Shim behavior).
    
    switch (sig) {
        case SIGKILL: // Hard kill
            printf("   [BSD] SIGKILL -> task_terminate()\n");
            task_terminate(target_task);
            break;
            
        case SIGTERM: // Terminate (graceful) - for now implementation same as kill
            printf("   [BSD] SIGTERM -> task_terminate()\n");
            task_terminate(target_task);
            break;
            
        case SIGSTOP: // Suspend
            printf("   [BSD] SIGSTOP -> task_suspend()\n");
            task_suspend(target_task);
            break;
            
        case SIGCONT: // Resume
            printf("   [BSD] SIGCONT -> task_resume()\n");
            task_resume(target_task);
            break;
            
        default:
            printf("   [BSD] Signal %d not implemented yet, ignoring.\n", sig);
            return 0;
    }
    
    return 0;
}

int bsd_signal(int sig, sighandler_t handler) {
    (void)sig; (void)handler;
    // Registering handlers would require modifying the Process Control Block to store handlers
    // For now we only support the default actions above.
    return 0;
}
