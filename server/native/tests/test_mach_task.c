#include "mach/task.h"
#include <stdio.h>
#include <unistd.h>

void scheduler_init(void);

void test_mach_task_logic() {
    printf("🍏 Mach Task Test\n");
    printf("=================\n");
    
    scheduler_init();
    
    // 1. Create Task
    printf("🧪 Test 1: Task Creation\n");
    task_t task = task_create();
    
    if (task != MACH_TASK_NULL) {
        printf("✅ Task created: handle=%d PID=%d\n", task, task_pid(task));
    } else {
        printf("❌ Task creation failed\n");
        return;
    }
    
    // 2. Suspend/Resume
    printf("\n🧪 Test 2: Suspend/Resume\n");
    task_suspend(task);
    // Verify state somehow? We generally rely on logs here.
    // In real unit test we would check scheduler_get_process(pid)->state
    
    task_resume(task);
    
    // 3. Terminate
    printf("\n🧪 Test 3: Termination\n");
    int pid = task_pid(task);
    task_terminate(task);
    
    // Verify it's gone from task API
    if (task_pid(task) == -1) {
        printf("✅ Task handle invalidated\n");
    } else {
        printf("❌ Task handle still valid\n");
    }
    
    printf("\n✅ Mach Task tests completed\n");
}

int main() {
    test_mach_task_logic();
    return 0;
}
