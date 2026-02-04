#include "usr_common.h"

// Forward decl
void shell_main(); 

void init_main() {
    const char* msg = "\n[Init] System Initialized (PID 1)\n";
    sys_write(1, msg, strlen(msg));
    
    // 1. Mount additional FS? (Done in kernel for now)
    
    // 2. Start Services
    // In a logical OS, we'd start a logger daemon, etc.
    
    // 3. Spawn Shell
    while (1) {
        sys_write(1, "[Init] Spawning Shell...\n", 25);
        // Use 64KB stack for shell
        int pid = scheduler_create_process(shell_main, 64 * 1024); 
        
        if (pid > 0) {
            // Wait for shell to exit
            int status;
            // sys_wait4(pid, &status, 0, NULL); 
            // Mock waiting loop as sys_wait4 isn't blocking yet in our sim
            while (1) {
                 process_t* proc = scheduler_get_process(pid);
                 if (!proc || proc->state == PROCESS_TERMINATED) break;
                 scheduler_yield();
                 // sleep(1); // don't sleep in kernel thread, just yield
            }
            
            sys_write(1, "[Init] Shell exited. Respawning in 1s...\n", 41);
        }
        
        // Simple delay loop
        for (volatile int i=0; i<10000000; i++);
    }
}
