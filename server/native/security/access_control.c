#include "security.h"
#include "../kernel/scheduler.h"
#include <stdio.h>
#include <string.h>

// Check if a process has a specific capability
bool security_check_capability(process_t* proc, uint32_t cap) {
    if (!proc) return false;
    
    // Root (UID 0) implicitly has capabilities? 
    // Usually capability systems decouple UID 0 from power, but for now we assume UID 0 or having the bit set.
    if (proc->security.uid == 0) return true;
    
    return (proc->security.capabilities & cap) != 0;
}

// Check permission to open a file
// Mode: 4=Read, 2=Write, 1=Exec
bool security_can_open_file(process_t* proc, const char* path, int mode) {
    if (!proc) return false;
    if (!path) return false;
    
    // 1. Root Bypass
    if (proc->security.uid == 0 || security_check_capability(proc, CAP_DAC_OVERRIDE)) {
        return true; 
    }
    
    // 2. Mock Logic:
    // "sensitive.txt" is root-only
    // "dev/" is root-only write
    
    if (strstr(path, "sensitive.txt")) {
        // Only root can access
        // Since we passed 1., we know we are NOT root.
        // So deny.
        printf("🛑 Security Denial: PID %d (UID %d) tried to open '%s'\n", 
               proc->pid, proc->security.uid, path);
        return false;
    }
    
    // Allow everything else for now (RamFS is open)
    return true;
}

// Check if process can signal another process
bool security_can_kill(process_t* target, process_t* caller) {
    if (!target || !caller) return false;
    
    if (caller->security.uid == 0 || security_check_capability(caller, CAP_KILL)) {
        return true;
    }
    
    // Same UID check
    if (caller->security.uid == target->security.uid) {
        return true;
    }
    
    printf("🛑 Security Denial: PID %d (UID %d) tried to kill PID %d (UID %d)\n",
           caller->pid, caller->security.uid, target->pid, target->security.uid);
           
    return false;
}
