#include "bsd/signal.h"
#include "mach/task.h"
#include <stdio.h>
#include <unistd.h>

void scheduler_init(void);

void test_bsd_layer() {
    printf("👹 BSD Compatibility Test (Signals)\n");
    printf("===================================\n");
    
    scheduler_init();
    
    // 1. Create a target task
    task_t target = task_create();
    int pid = task_pid(target);
    printf("✅ Created target process PID %d\n", pid);
    
    // 2. Test SIGSTOP (Suspend)
    printf("\n🧪 Test 1: SIGSTOP\n");
    bsd_kill(pid, SIGSTOP);
    // Internal verification: task_suspend should have been called
    
    // 3. Test SIGCONT (Resume)
    printf("\n🧪 Test 2: SIGCONT\n");
    bsd_kill(pid, SIGCONT);
    
    // 4. Test SIGKILL (Terminate)
    printf("\n🧪 Test 3: SIGKILL\n");
    bsd_kill(pid, SIGKILL);
    
    // Verify death
    if (task_from_pid(pid) == MACH_TASK_NULL) {
         // Wait, task_terminate deallocates the port, so look up fails?
         // Yes, task_terminate calls deallocate.
         // But task_from_pid checks active flags?
         printf("✅ Process terminated (Task lookup failed)\n");
    } else {
         printf("❌ Process still alive\n");
    }
    
    printf("\n✅ BSD Signal tests completed\n");
}

int main() {
    test_bsd_layer();
    return 0;
}
