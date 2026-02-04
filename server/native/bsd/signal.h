#ifndef HARMONY_BSD_SIGNAL_H
#define HARMONY_BSD_SIGNAL_H

#include <stdint.h>
#include <sys/types.h> // for pid_t

// POSIX Signal Numbers (standard subset)
#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGEMT      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGBUS      10
#define SIGSEGV     11
#define SIGSYS      12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGURG      16
#define SIGSTOP     17
#define SIGTSTP     18
#define SIGCONT     19
#define SIGCHLD     20

// Signal Action Structure
typedef void (*sighandler_t)(int);

struct sigaction {
    sighandler_t sa_handler;
    uint32_t     sa_mask;
    int          sa_flags;
};

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

// API
int bsd_kill(int pid, int sig);
int bsd_signal(int sig, sighandler_t handler);

#endif // HARMONY_BSD_SIGNAL_H
