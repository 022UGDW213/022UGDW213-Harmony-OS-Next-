#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "boot/hardware.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"
#include "kernel/scheduler.h"
#include "bridge.h"
#include "kernel/syscall.h"
#include "rust/rust_fs.h"
#include "rust/rust_net.h"
#include "kernel/log.h"

// External init process (from usr/init.c)
extern void init_main(void);

// --- Kernel Init Process ---
// This acts as the "init" process (PID 1)
void init_process(void) {
    printf("\n🚀 Init Process (PID 1) Started\n");
    printf("--------------------------------\n");
    
    // 1. Filesystem Check
    printf("[Init] Mounting filesystems...\n");
    if (rust_fs_init() == 0) {
        printf("       ✅ RamFS mounted at /\n");
        // Create some default files
        int fd = rust_fs_open("/welcome.msg", 1 | 4); // O_CREAT | O_WRONLY
        if (fd > 0) {
            const char* msg = "Welcome to HarmonyOS!";
            rust_fs_write(fd, msg, strlen(msg));
            rust_fs_close(fd);
            printf("       📝 Created /welcome.msg\n");
        }
    } else {
        printf("       ❌ Filesystem mount failed\n");
    }
    
    // 2. Network Bring-up
    printf("[Init] Initializing loopback interface...\n");
    rust_net_init(); // Just initializes the stack structures in our mock
    printf("       ✅ lo0 up (127.0.0.1)\n");
    
    // 3. User Space Simulation
    printf("[Init] Entering user mode loop...\n");
    
    for (int i = 0; i < 3; i++) {
        printf("       🔄 System active... tick %d\n", i);
        
        // Demonstrate System Call from "User Mode"
        const char* log = "Log entry from init process\n";
        sys_write(1, log, strlen(log));
        
        scheduler_yield(); // Cooperative yield
        sleep(1);
    }
    
    printf("[Init] System shutdown requested.\n");
    scheduler_exit_process(0);
}

// --- Main Kernel Entry Point ---
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0); // Disable output buffering
    
    // 0. Initialize Logger
    klog_init();
    
    printf("\n");
    printf("🌌 HarmonyOS Kernel Booting...\n");
    printf("=============================\n");
    klog(KLOG_INFO, "HarmonyOS Kernel Booting...\n");
    
    // 1. Hardware Detection
    hardware_detect();
    klog(KLOG_INFO, "Hardware Detected\n");
    
    // 2. Architecture Init (Interrupts)
    printf("🔧 Initializing Interrupt Descriptor Table (IDT)...\n");
    interrupts_init();
    
    // 3. Syscall Subsystem Init
    printf("📞 Initializing Syscall Subsystem...\n");
    syscall_init();
    
    // 4. Memory Management Init
    printf("💾 Initializing Memory Manager...\n");
    // Simulate memory map passed from bootloader
    // Assuming 64MB managed by our buddy allocator (Phase 1)
    memory_manager_t* mm = memory_init(64 * 1024 * 1024);
    (void)mm; // Keep the pointer for future use
    printf("   ✅ Physical Page Allocator Ready\n");
    printf("   ✅ Virtual Memory Manager Ready\n");

    // 4. Scheduler Init
    printf("⚙️  Initializing Process Scheduler...\n");
    scheduler_init();
    
    // 5. Handover to First Process
    printf("🏃 Spawning Init Process (PID 1)...\n");
    int pid = scheduler_create_process(init_main, 8192);
    if (pid < 0) {
        printf("❌ Failed to spawn init!\n");
        return 1;
    }
    
    // 6. Initialize Bridge (IPC)
    bridge_init();

    printf("✅ Boot Sequence Complete. Transforming to Scheduler...\n");
    printf("====================================================\n\n");
    
    // Start multitasking (Does not return)
    schedule();
    
    // We should only reach here if scheduler exits (all processes dead)
    printf("🛑 Kernel Halted.\n");
    return 0;
}
