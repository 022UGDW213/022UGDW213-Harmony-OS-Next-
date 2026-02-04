#include "usr_common.h"

// --- FPU Test Processes ---
void fpu_process_a() {
    float a = 1.5f;
    char msg[64];
    for (int i = 0; i < 5; i++) {
        a += 1.1f;
        // Expected: 1.5, 2.6, 3.7, 4.8, 5.9, 7.0
        sprintf(msg, "[PROC A] Float: %.2f\n", a);
        sys_write(1, msg, strlen(msg));
        
        // Burn cycles
        for(volatile int k=0; k<100000; k++);
        scheduler_yield();
    }
    sys_exit(0);
}

void fpu_process_b() {
    double b = 100.0;
    char msg[64];
    for (int i = 0; i < 5; i++) {
        b /= 2.0;
        // Expected: 100, 50, 25, 12.5, 6.25, 3.125
        sprintf(msg, "[PROC B] Double: %.2f\n", b);
        sys_write(1, msg, strlen(msg));
        
        for(volatile int k=0; k<100000; k++);
        scheduler_yield();
    }
    sys_exit(0);
}

// Shell Commands
void cmd_help() {
    char msg[] = "\nAvailable Commands:\n"
                 "  ls [path]   List directory\n"
                 "  ps          List processes\n"
                 "  cat [file]  Read file\n"
                 "  mem         Show memory stats\n"
                 "  kill [pid]  Kill process\n"
                 "  whoami      Show current UID\n"
                 "  su [uid]    Switch User ID\n"
                 "  help        Show this message\n"
                 "  exit        Exit shell\n";
    sys_write(1, msg, sizeof(msg)-1);
}

void cmd_ls(const char* path) {
    // In a real User Space, we'd use opendir/readdir (via Syscalls)
    // Here we have to cheat slightly and call the mock FS directly 
    // OR implement sys_getdents.
    // For now, let's use a dummy message as we didn't implement sys_getdents yet.
    
    char msg[] = "Listing: /\n  welcome.msg\n  bin/\n  dev/\n";
    sys_write(1, msg, sizeof(msg)-1);
    
    // TODO: Connect to rust_fs_list via syscall
}

void cmd_ps() {
    scheduler_print_processes();
}

void cmd_cat(const char* file) {
    int fd = sys_open(file, 0, 0); // O_RDONLY
    if (fd < 0) {
        char err[] = "Error: File not found\n";
        sys_write(1, err, sizeof(err)-1);
        return;
    }
    
    char buf[128];
    int bytes;
    while ((bytes = sys_read(fd, buf, sizeof(buf))) > 0) {
        sys_write(1, buf, bytes);
    }
    sys_close(fd);
    sys_write(1, "\n", 1);
}

void cmd_whoami() {
    // We need to implement sys_getuid syscall wrapper locally if not in usr_common
    // For now we use the raw syscall via wrapper we'll assume exist or mock
    // Wait, usr code relies on calling sys_* functions which are *kernel* functions directly 
    // linked in this simulation. In a real OS we'd use asm syscalls.
    // So we can just call sys_getuid().
    
    int uid = sys_getuid();
    char msg[64];
    // Simple itoa
    sprintf(msg, "UID: %d\n", uid);
    sys_write(1, msg, strlen(msg));
}

void cmd_su(int uid) {
    if (sys_setuid(uid) == 0) {
        char msg[] = "Switched User.\n";
        sys_write(1, msg, sizeof(msg)-1);
    } else {
        char msg[] = "Start failed: Permission Denied.\n";
        sys_write(1, msg, sizeof(msg)-1);
    }
}

void shell_main() {
    char prompt[] = "harmony$ ";
    char input[256];
    
    sys_write(1, "\nWelcome to Harmony Shell (hsh)\n", 32);
    
    // Auto-drop logic for demo?
    // Let's stay root initially so user can explore, but adding 'whoami' helps.
    
    cmd_help();
    
    cmd_help();
    
    while (1) {
        sys_write(1, prompt, strlen(prompt));
        
        // Read line char-by-char to support piped input
        int pos = 0;
        char c;
        while (pos < sizeof(input) - 1) {
            int n = sys_read(0, &c, 1);
            if (n <= 0) break;
            if (c == '\n') break;
            input[pos++] = c;
        }
        input[pos] = 0;
        
        if (pos == 0 && c != '\n') break; // EOF
        
        if (strlen(input) == 0) continue;
        
        if (strcmp(input, "help") == 0) {
            cmd_help();
        } else if (strncmp(input, "ls", 2) == 0) {
            cmd_ls("/");
        } else if (strncmp(input, "ps", 2) == 0) {
            cmd_ps();
        } else if (strncmp(input, "cat ", 4) == 0) {
            cmd_cat(input + 4);
        } else if (strcmp(input, "whoami") == 0) {
            cmd_whoami();
        } else if (strcmp(input, "dmesg") == 0) {
            char logbuf[4096];
            // Mock syscall wrapper
            sys_dmesg(logbuf, sizeof(logbuf));
            sys_write(1, logbuf, strlen(logbuf));
            sys_write(1, "\n", 1);
        } else if (strcmp(input, "top") == 0) {
            // Enhanced PS
             sys_write(1, "\033[2J\033[H", 7); // Clear screen
             scheduler_print_processes();
             // In real top we'd loop. Here just once.
        } else if (strcmp(input, "qstat") == 0) {
             char q[] = "Quantum Matrix Status (Mock View):\n[PID 1] CPU:0.5 IO:0.5\n[PID 2] CPU:0.8 IO:0.2\n";
             sys_write(1, q, strlen(q));
        } else if (strcmp(input, "fpu") == 0) {
             char msg[] = "🚀 Spawning FPU Test Processes...\n";
             sys_write(1, msg, strlen(msg));
             scheduler_create_process(fpu_process_a, 16384);
             scheduler_create_process(fpu_process_b, 16384);
        } else if (strcmp(input, "cow") == 0) {
             // Mock CoW test
             void* addr = sys_mmap(NULL, 4096, 1|2, 0x22, -1, 0); // RW
             // Mark COW (Simulation via permission change if we had sys_mprotect)
             // Since we don't have sys_mprotect exposed yet, we rely on kernel log from mmap if we passed COW flag?
             // Let's just print what would happen:
             char msg[] = "🐮 COW Simulation:\n1. Parent Maps 0x1000 [RW]\n2. Fork() -> Child Shares 0x1000 [COW]\n3. Child Writes -> Page Fault -> Clone Page\n";
             sys_write(1, msg, strlen(msg));
        } else if (strncmp(input, "su ", 3) == 0) {
            // parse uid
            int uid = 1000; // default for demo if parse fails
            // simple parse
            if (input[3] >= '0' && input[3] <= '9') {
                uid = input[3] - '0';
                if (input[4] >= '0' && input[4] <= '9') {
                     uid = uid * 10 + (input[4]-'0');
                }
            }
            cmd_su(uid);
        } else if (strcmp(input, "mem") == 0) {
            // Not accessible via syscall yet
             char msg[] = "Use 'ps' to see basic stats.\n";
             sys_write(1, msg, strlen(msg));
        } else if (strcmp(input, "exit") == 0) {
            break;
        } else {
            char err[] = "Unknown command.\n";
            sys_write(1, err, sizeof(err)-1);
        }
        
        scheduler_yield();
    }
    
    sys_exit(0);
}
