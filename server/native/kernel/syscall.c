#include "syscall.h"
#include "memory.h"
#include "scheduler.h"
#include "scheduler.h"
#include "../security/access_control.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Global syscall statistics
static syscall_stats_t g_syscall_stats = {0};

// File descriptor table (simplified)
#define MAX_FDS 256
static int g_fd_table[MAX_FDS];
static int g_next_fd = 3; // Start after stdin/stdout/stderr

// Memory manager reference (set during init)
static memory_manager_t* g_memory_manager = NULL;

// Initialize syscall subsystem
void syscall_init(void) {
    memset(&g_syscall_stats, 0, sizeof(syscall_stats_t));
    
    // Initialize FD table
    for (int i = 0; i < MAX_FDS; i++) {
        g_fd_table[i] = -1;
    }
    
    // Standard file descriptors
    g_fd_table[STDIN_FILENO] = STDIN_FILENO;
    g_fd_table[STDOUT_FILENO] = STDOUT_FILENO;
    g_fd_table[STDERR_FILENO] = STDERR_FILENO;
    
    printf("🔧 Syscall subsystem initialized\n");
}

// Shutdown syscall subsystem
void syscall_shutdown(void) {
    printf("🔧 Syscall subsystem shutdown\n");
}

// Set memory manager for syscalls
void syscall_set_memory_manager(memory_manager_t* mm) {
    g_memory_manager = mm;
}

// Main syscall dispatcher
long syscall_handler(long syscall_num, long arg1, long arg2, long arg3, long arg4, long arg5) {
    g_syscall_stats.total_calls++;
    
    // printf("📞 Syscall %ld called\n", syscall_num);
    // klog(KLOG_DEBUG, "Syscall %ld\n", syscall_num); // Too verbose for default
    
    switch (syscall_num) {
        case SYS_READ:
            g_syscall_stats.read_calls++;
            return sys_read((int)arg1, (void*)arg2, (size_t)arg3);
            
        case SYS_WRITE:
            g_syscall_stats.write_calls++;
            return sys_write((int)arg1, (const void*)arg2, (size_t)arg3);
            
        case SYS_OPEN:
            g_syscall_stats.open_calls++;
            return sys_open((const char*)arg1, (int)arg2, (int)arg3);
            
        case SYS_CLOSE:
            g_syscall_stats.close_calls++;
            return sys_close((int)arg1);
            
        case SYS_MMAP:
            g_syscall_stats.mmap_calls++;
            return (long)sys_mmap((void*)arg1, (size_t)arg2, (int)arg3, (int)arg4, (int)arg5, 0);
            
        case SYS_MUNMAP:
            return sys_munmap((void*)arg1, (size_t)arg2);
            
        case SYS_BRK:
            return (long)sys_brk((void*)arg1);
            
        case SYS_FORK:
            g_syscall_stats.fork_calls++;
            return sys_fork();
            
        case SYS_EXECVE:
            g_syscall_stats.exec_calls++;
            return sys_execve((const char*)arg1, (char**)arg2, (char**)arg3);
            
        case SYS_EXIT:
            g_syscall_stats.exit_calls++;
            sys_exit((int)arg1);
            return 0; // Never reached
            
        case SYS_WAIT4:
            return sys_wait4((int)arg1, (int*)arg2, (int)arg3, (void*)arg4);

        case 102: // SYS_GETUID (Custom number for now, Linux is 102)
             return sys_getuid();
        
        case 105: // SYS_SETUID (Custom/Linux)
             return sys_setuid((uid_t)arg1);

        case 110: // SYS_DMESG (Custom)
             return sys_dmesg((char*)arg1, (size_t)arg2);
            
        default:
            fprintf(stderr, "⚠️  Unknown syscall: %ld\n", syscall_num);
            return -EINVAL;
    }
}

// ============================================================================
// PROCESS SYSCALLS
// ============================================================================

int sys_fork(void) {
    printf("🍴 sys_fork() called\n");
    // TODO: Implement actual fork logic
    // For now, return a dummy PID
    return 1234;
}

int sys_execve(const char* path, char** argv, char** envp) {
    printf("🚀 sys_execve(\"%s\") called\n", path ? path : "NULL");
    // TODO: Implement actual exec logic
    return 0;
}

void sys_exit(int status) {
    printf("🚪 sys_exit(%d) called\n", status);
    // TODO: Cleanup process resources
    exit(status);
}

int sys_wait4(int pid, int* status, int options, void* rusage) {
    printf("⏳ sys_wait4(%d) called\n", pid);
    // TODO: Implement actual wait logic
    if (status) {
        *status = 0;
    }
    return pid;
}

#include "log.h"
int sys_dmesg(char* buf, size_t size) {
    printf("📜 sys_dmesg(%p, %zu)\n", buf, size);
    return sys_dmesg_read(buf, size);
}

// ============================================================================
// FILE SYSCALLS
// ============================================================================

int sys_open(const char* path, int flags, int mode) {
    if (!path) {
        return -EFAULT;
    }
    
    printf("📂 sys_open(\"%s\", 0x%x, 0%o)\n", path, flags, mode);

    // SECURE: Check permissions
    process_t* current = scheduler_get_current_process();
    if (!security_can_open_file(current, path, 4)) { // 4=Read
        klog(KLOG_WARN, "Security Denial: PID %d (UID %d) tried to open '%s'\n", 
             current->pid, current->security.uid, path);
        return -13; // EACCES
    }
    
    // Find free FD
    if (g_next_fd >= MAX_FDS) {
        return -ENOMEM;
    }
    
    // Actually open the file (using host OS for now)
    int host_fd = open(path, flags, mode);
    if (host_fd < 0) {
        return -EBADF;
    }
    
    int fd = g_next_fd++;
    g_fd_table[fd] = host_fd;
    
    printf("   → Allocated FD %d (host FD %d)\n", fd, host_fd);
    return fd;
}

ssize_t sys_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_FDS || g_fd_table[fd] < 0) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EFAULT;
    }
    
    printf("📖 sys_read(%d, %p, %zu)\n", fd, buf, count);
    
    // Use host OS read for now
    ssize_t bytes = read(g_fd_table[fd], buf, count);
    
    printf("   → Read %zd bytes\n", bytes);
    return bytes;
}

ssize_t sys_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_FDS || g_fd_table[fd] < 0) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EFAULT;
    }
    
    printf("✍️  sys_write(%d, %p, %zu)\n", fd, buf, count);
    
    // Use host OS write for now
    ssize_t bytes = write(g_fd_table[fd], buf, count);
    
    printf("   → Wrote %zd bytes\n", bytes);
    return bytes;
}

int sys_close(int fd) {
    if (fd < 0 || fd >= MAX_FDS || g_fd_table[fd] < 0) {
        return -EBADF;
    }
    
    printf("🔒 sys_close(%d)\n", fd);
    
    // Close host FD
    int result = close(g_fd_table[fd]);
    g_fd_table[fd] = -1;
    
    return result;
}

// ============================================================================
// MEMORY SYSCALLS
// ============================================================================

void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    printf("🗺️  sys_mmap(%p, %zu, 0x%x, 0x%x, %d, %ld)\n", 
           addr, length, prot, flags, fd, (long)offset);
    
    if (!g_memory_manager) {
        fprintf(stderr, "⚠️  Memory manager not initialized\n");
        return (void*)-ENOMEM;
    }
    
    // Convert prot flags to memory protection
    uint32_t mem_prot = 0;
    if (prot & 0x1) mem_prot |= MEM_PROT_READ;   // PROT_READ
    if (prot & 0x2) mem_prot |= MEM_PROT_WRITE;  // PROT_WRITE
    if (prot & 0x4) mem_prot |= MEM_PROT_EXEC;   // PROT_EXEC
    
    // Allocate virtual memory
    void* result = alloc_virtual(g_memory_manager, addr, length, mem_prot);
    
    if (!result) {
        return (void*)-ENOMEM;
    }
    
    printf("   → Mapped at %p\n", result);
    return result;
}

int sys_munmap(void* addr, size_t length) {
    printf("🗑️  sys_munmap(%p, %zu)\n", addr, length);
    
    if (!g_memory_manager) {
        return -EINVAL;
    }
    
    bool success = free_virtual(g_memory_manager, addr, length);
    return success ? 0 : -EINVAL;
}

void* sys_brk(void* addr) {
    printf("📊 sys_brk(%p)\n", addr);
    
    // TODO: Implement heap management
    // For now, just return the requested address
    return addr;
}

// ============================================================================
// SECURITY SYSCALLS (PHASE 8)
// ============================================================================

int sys_getuid(void) {
    process_t* current = scheduler_get_current_process();
    if (!current) return -1;
    return current->security.uid;
}

int sys_setuid(uid_t uid) {
    process_t* current = scheduler_get_current_process();
    if (!current) return -1;
    
    // Only Root or CAP_SETUID can change UID
    if (current->security.uid == 0 || security_check_capability(current, CAP_SETUID)) {
        printf("🔐 PID %d Changed UID %d -> %d\n", current->pid, current->security.uid, uid);
        current->security.uid = uid;
        // Simplified: Clear caps if dropping from root? 
        // For strictness, if switching to non-zero, we should drop caps unless kept.
        if (uid != 0) {
            current->security.capabilities = 0; 
        }
        return 0;
    }
    
    printf("⛔ PID %d (UID %d) failed to setuid(%d) - Permission Denied\n", 
           current->pid, current->security.uid, uid);
           
    return -1; // EPERM
}

// ============================================================================
// STATISTICS
// ============================================================================

syscall_stats_t get_syscall_stats(void) {
    return g_syscall_stats;
}

void print_syscall_stats(void) {
    printf("\n📊 Syscall Statistics:\n");
    printf("=====================\n");
    printf("Total calls:  %llu\n", g_syscall_stats.total_calls);
    printf("\nBreakdown:\n");
    printf("  read:       %llu\n", g_syscall_stats.read_calls);
    printf("  write:      %llu\n", g_syscall_stats.write_calls);
    printf("  open:       %llu\n", g_syscall_stats.open_calls);
    printf("  close:      %llu\n", g_syscall_stats.close_calls);
    printf("  mmap:       %llu\n", g_syscall_stats.mmap_calls);
    printf("  fork:       %llu\n", g_syscall_stats.fork_calls);
    printf("  exec:       %llu\n", g_syscall_stats.exec_calls);
    printf("  exit:       %llu\n", g_syscall_stats.exit_calls);
    printf("\n");
}
