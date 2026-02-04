#ifndef HARMONY_SECURITY_H
#define HARMONY_SECURITY_H

#include <stdint.h>
#include <stdbool.h>

// User and Group IDs
typedef uint32_t uid_t;
typedef uint32_t gid_t;

// Capability Flags (Linux-inspired subset)
#define CAP_CHOWN           (1 << 0)
#define CAP_DAC_OVERRIDE    (1 << 1) // Bypass file permission checks
#define CAP_DAC_READ_SEARCH (1 << 2) // Bypass read permission checks
#define CAP_FOWNER          (1 << 3)
#define CAP_KILL            (1 << 4) // Bypass process permission checks
#define CAP_SETGID          (1 << 5)
#define CAP_SETUID          (1 << 6)
#define CAP_NET_BIND_SERVICE (1 << 10)
#define CAP_NET_ADMIN       (1 << 12)
#define CAP_SYS_ADMIN       (1 << 21) // "God mode"

// Security Context attached to each process
typedef struct {
    uid_t uid;              // Real User ID
    gid_t gid;              // Real Group ID
    uint32_t capabilities;  // E.g. CAP_SYS_ADMIN
} security_context_t;

// Initial Root Context
static const security_context_t ROOT_CONTEXT = { .uid = 0, .gid = 0, .capabilities = 0xFFFFFFFF };
static const security_context_t USER_CONTEXT = { .uid = 1000, .gid = 1000, .capabilities = 0 };

#endif // HARMONY_SECURITY_H
