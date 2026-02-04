#ifndef HARMONY_LINUX_NAMESPACE_H
#define HARMONY_LINUX_NAMESPACE_H

#include <stdint.h>
#include <stdbool.h>

// PID Namespace
typedef struct pid_namespace {
    int level;
    struct pid_namespace* parent;
    int last_pid;
    // Real implementation needs a PID map (pid_t local -> pid_t global)
} pid_namespace_t;

// Proxy Structure (held by process)
typedef struct nsproxy {
    pid_namespace_t* pid_ns;
    // Add other namespaces later (mnt, net, user...)
} nsproxy_t;

// API
nsproxy_t* create_nsproxy(void);
pid_namespace_t* create_pid_namespace(pid_namespace_t* parent);
void free_nsproxy(nsproxy_t* ns);

// PID Logic
int alloc_pid(pid_namespace_t* ns);

#endif // HARMONY_LINUX_NAMESPACE_H
