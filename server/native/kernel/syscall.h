#ifndef HARMONY_SYSCALL_H
#define HARMONY_SYSCALL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// System call numbers (Linux-compatible)
#define SYS_READ        0
#define SYS_WRITE       1
#define SYS_OPEN        2
#define SYS_CLOSE       3
#define SYS_STAT        4
#define SYS_FSTAT       5
#define SYS_MMAP        9
#define SYS_MUNMAP      11
#define SYS_BRK         12
#define SYS_FORK        57
#define SYS_EXECVE      59
#define SYS_EXIT        60
#define SYS_WAIT4       61

// File flags (POSIX)
#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_CREAT         0x0040
#define O_TRUNC         0x0200
#define O_APPEND        0x0400

// File descriptors
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

// Error codes
#define EBADF           9   // Bad file descriptor
#define ENOMEM          12  // Out of memory
#define EFAULT          14  // Bad address
#define EINVAL          22  // Invalid argument

// Syscall statistics
typedef struct syscall_stats {
    uint64_t total_calls;
    uint64_t read_calls;
    uint64_t write_calls;
    uint64_t open_calls;
    uint64_t close_calls;
    uint64_t mmap_calls;
    uint64_t fork_calls;
    uint64_t exec_calls;
    uint64_t exit_calls;
} syscall_stats_t;

// Syscall handler initialization
void syscall_init(void);
void syscall_shutdown(void);

// Main syscall dispatcher
long syscall_handler(long syscall_num, long arg1, long arg2, long arg3, long arg4, long arg5);

// Process syscalls
int sys_fork(void);
int sys_execve(const char* path, char** argv, char** envp);
void sys_exit(int status);
int sys_wait4(int pid, int* status, int options, void* rusage);

// Security
int sys_getuid(void);
int sys_setuid(uid_t uid);
int sys_dmesg(char* buf, size_t size);

// File syscalls
int sys_open(const char* path, int flags, int mode);
ssize_t sys_read(int fd, void* buf, size_t count);
ssize_t sys_write(int fd, const void* buf, size_t count);
int sys_close(int fd);

// Memory syscalls
void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int sys_munmap(void* addr, size_t length);
void* sys_brk(void* addr);

// Statistics
syscall_stats_t get_syscall_stats(void);
void print_syscall_stats(void);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_SYSCALL_H
