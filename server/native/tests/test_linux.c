#include "linux/namespace.h"
#include "kernel/scheduler.h"
#include <stdio.h>

// We need to extern scheduler internal if we want to attach nsproxy manually
// Or we add an API: scheduler_attach_namespace(pid, nsproxy)

void scheduler_init(void);

// Helper from scheduler.c (if not exposed, needed for test setup)
extern process_t* scheduler_get_process(int pid);

void test_linux_ns() {
    printf("🐧 Linux Namespace Test\n");
    printf("=======================\n");
    
    scheduler_init();
    
    // 1. Create Root Namespace
    pid_namespace_t* root_ns = create_pid_namespace(NULL);
    if (!root_ns) {
        printf("❌ Failed to create root NS\n");
        return;
    }
    
    // 2. Create a "Container" Namespace
    pid_namespace_t* container_ns = create_pid_namespace(root_ns);
    
    // 3. Create a Process inside the Container
    // In real Linux, we fork/clone with CLONE_NEWPID.
    // Here we create a process, then attach the NS.
    
    int global_pid = scheduler_create_process(NULL, 0); // Dummy entry
    process_t* proc = scheduler_get_process(global_pid);
    
    if (proc) {
        proc->nsproxy = create_nsproxy();
        proc->nsproxy->pid_ns = container_ns;
        
        // Allocate Local PID
        int local_pid = alloc_pid(container_ns);
        
        printf("✅ Process Created:\n");
        printf("   Global PID: %d\n", global_pid);
        printf("   Local PID:  %d (inside NS level %d)\n", local_pid, container_ns->level);
        
        if (local_pid == 1) {
            printf("✅ Validated: First process in new NS is PID 1\n");
        } else {
            printf("❌ Unexpected local PID: %d\n", local_pid);
        }
        
    } else {
        printf("❌ Failed to create process\n");
    }
    
    // 4. Create another process in same container
    int global_pid2 = scheduler_create_process(NULL, 0);
    process_t* proc2 = scheduler_get_process(global_pid2);
    if (proc2) {
         proc2->nsproxy = create_nsproxy();
         proc2->nsproxy->pid_ns = container_ns;
         int local_pid2 = alloc_pid(container_ns);
         printf("   Process 2: Global PID %d -> Local PID %d\n", global_pid2, local_pid2);
         if (local_pid2 == 2) printf("✅ Validated PID sequence\n");
    }
    
    printf("\n✅ Linux Namespace tests completed\n");
}

int main() {
    test_linux_ns();
    return 0;
}
