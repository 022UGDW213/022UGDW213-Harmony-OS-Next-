#include "linux/namespace.h"
#include <stdlib.h>
#include <stdio.h>

pid_namespace_t* create_pid_namespace(pid_namespace_t* parent) {
    pid_namespace_t* ns = (pid_namespace_t*)malloc(sizeof(pid_namespace_t));
    if (!ns) return NULL;
    
    ns->parent = parent;
    ns->level = parent ? parent->level + 1 : 0;
    ns->last_pid = 0;
    
    printf("   [Linux] Created PID Namespace (Level %d)\n", ns->level);
    return ns;
}

nsproxy_t* create_nsproxy(void) {
    nsproxy_t* proxy = (nsproxy_t*)malloc(sizeof(nsproxy_t));
    if (!proxy) return NULL;
    
    // Default to a root PID namespace if none provided?
    // For now we initialize manualy.
    proxy->pid_ns = NULL;
    
    return proxy;
}

void free_nsproxy(nsproxy_t* proxy) {
    if (!proxy) return;
    free(proxy);
    // Note: Namespaces are refcounted in real Linux. Here we don't own the NS directly via this pointer.
}

int alloc_pid(pid_namespace_t* ns) {
    if (!ns) return -1;
    // Simple bump allocator per namespace
    return ++ns->last_pid;
}
