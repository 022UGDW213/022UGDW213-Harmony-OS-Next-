#ifndef HARMONY_MACH_PORT_H
#define HARMONY_MACH_PORT_H

#include <stdint.h>
#include <stdbool.h>

// Mach Port Type (Integer Handle)
typedef uint32_t mach_port_t;

#define MACH_PORT_NULL      0
#define MACH_PORT_DEAD      ((mach_port_t)~0)

// Port Rights
#define MACH_PORT_RIGHT_SEND    0
#define MACH_PORT_RIGHT_RECEIVE 1

// Internal Port Object (kernel side)
typedef struct ipc_port {
    int id;                     // Global ID
    int ref_count;              // Reference counting
    struct ipc_port* next;      // Linked list for global table
    // Message queue will be added here
    void* message_queue;
} ipc_port_t;

// API
mach_port_t mach_port_allocate(void);
void mach_port_deallocate(mach_port_t port);
bool mach_port_valid(mach_port_t port);

#endif // HARMONY_MACH_PORT_H
