#include "kernel/syscall.h"
#include "kernel/memory.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

// Forward declaration
extern void syscall_set_memory_manager(memory_manager_t* mm);

void test_syscalls() {
    printf("🔧 System Call Layer Test\n");
    printf("===========================\n\n");
    
    // Initialize syscall subsystem
    syscall_init();
    
    // Initialize memory manager for mmap tests
    memory_manager_t* mm = memory_init(64 * 1024 * 1024);
    syscall_set_memory_manager(mm);
    
    // Test 1: sys_write (most basic syscall)
    printf("🧪 Test 1: sys_write\n");
    printf("--------------------\n");
    const char* msg = "Hello from syscall!\n";
    ssize_t written = syscall_handler(SYS_WRITE, STDOUT_FILENO, (long)msg, strlen(msg), 0, 0);
    printf("Wrote %zd bytes\n\n", written);
    
    // Test 2: sys_open, sys_write, sys_close
    printf("🧪 Test 2: File Operations\n");
    printf("---------------------------\n");
    int fd = syscall_handler(SYS_OPEN, (long)"/tmp/test_syscall.txt", 
                            O_CREAT | O_WRONLY | O_TRUNC, 0644, 0, 0);
    printf("Opened file with FD: %d\n", fd);
    
    if (fd >= 0) {
        const char* file_msg = "Testing file write syscall\n";
        written = syscall_handler(SYS_WRITE, fd, (long)file_msg, strlen(file_msg), 0, 0);
        printf("Wrote %zd bytes to file\n", written);
        
        int close_result = syscall_handler(SYS_CLOSE, fd, 0, 0, 0, 0);
        printf("Close result: %d\n", close_result);
    }
    printf("\n");
    
    // Test 3: sys_mmap (memory allocation via syscall)
    printf("🧪 Test 3: sys_mmap\n");
    printf("-------------------\n");
    void* mapped = (void*)syscall_handler(SYS_MMAP, 0, 8192, 
                                          0x3, // PROT_READ | PROT_WRITE
                                          0, 0);
    printf("Mapped memory at: %p\n", mapped);
    
    if (mapped != (void*)-1) {
        // Write to mapped memory
        strcpy((char*)mapped, "Memory mapped via syscall!");
        printf("Wrote to mapped memory: %s\n", (char*)mapped);
        
        // Unmap
        int unmap_result = syscall_handler(SYS_MUNMAP, (long)mapped, 8192, 0, 0, 0);
        printf("Unmap result: %d\n", unmap_result);
    }
    printf("\n");
    
    // Test 4: sys_fork (stub)
    printf("🧪 Test 4: sys_fork\n");
    printf("-------------------\n");
    int pid = syscall_handler(SYS_FORK, 0, 0, 0, 0, 0);
    printf("Fork returned PID: %d\n", pid);
    printf("\n");
    
    // Test 5: sys_execve (stub)
    printf("🧪 Test 5: sys_execve\n");
    printf("---------------------\n");
    char* argv[] = {"/bin/echo", "hello", NULL};
    char* envp[] = {NULL};
    int exec_result = syscall_handler(SYS_EXECVE, (long)"/bin/echo", 
                                     (long)argv, (long)envp, 0, 0);
    printf("Exec result: %d\n", exec_result);
    printf("\n");
    
    // Test 6: sys_brk (heap management)
    printf("🧪 Test 6: sys_brk\n");
    printf("------------------\n");
    void* brk_result = (void*)syscall_handler(SYS_BRK, (long)0x1000000, 0, 0, 0, 0);
    printf("brk() returned: %p\n", brk_result);
    printf("\n");
    
    // Print statistics
    print_syscall_stats();
    
    // Cleanup
    memory_shutdown(mm);
    syscall_shutdown();
    
    printf("✅ All syscall tests completed!\n");
}

int main() {
    test_syscalls();
    return 0;
}
