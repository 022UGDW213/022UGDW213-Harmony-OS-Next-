#include "mach/port.h"
#include "mach/message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Simple global port table (simulated)
#define MAX_PORTS 1024
static struct {
    bool active;
    mach_msg_base_t* queue[16]; // Tiny message queue (buffer)
    int queue_head;
    int queue_tail;
    int queue_count;
} port_table[MAX_PORTS];

// Initialize subsystem
void mach_init(void) {
    memset(port_table, 0, sizeof(port_table));
    printf("🍎 Mach IPC Subsystem Initialized\n");
}

// Allocate a port
mach_port_t mach_port_allocate(void) {
    for (int i = 1; i < MAX_PORTS; i++) {
        if (!port_table[i].active) {
            port_table[i].active = true;
            port_table[i].queue_head = 0;
            port_table[i].queue_tail = 0;
            port_table[i].queue_count = 0;
            printf("   [Mach] Allocated Port %d\n", i);
            return (mach_port_t)i;
        }
    }
    return MACH_PORT_NULL;
}

// Deallocate
void mach_port_deallocate(mach_port_t port) {
    if (port > 0 && port < MAX_PORTS) {
        port_table[port].active = false;
        printf("   [Mach] Deallocated Port %d\n", port);
    }
}

// Send Message
int mach_msg_send(mach_msg_header_t* msg) {
    mach_port_t dest = msg->msgh_remote_port;
    
    if (dest <= 0 || dest >= MAX_PORTS || !port_table[dest].active) {
        printf("   [Mach] Send Failed: Invalid Dest Port %d\n", dest);
        return MACH_SEND_INVALID_DEST;
    }
    
    // Check queue space
    if (port_table[dest].queue_count >= 16) {
        printf("   [Mach] Send Failed: Queue Full for Port %d\n", dest);
        return MACH_MSG_IPC_ERROR;
    }
    
    // Copy message to queue
    mach_msg_base_t* copy = malloc(sizeof(mach_msg_base_t));
    if (!copy) return MACH_MSG_IPC_ERROR;
    
    memcpy(copy, msg, msg->msgh_size);
    
    int tail = port_table[dest].queue_tail;
    port_table[dest].queue[tail] = copy;
    port_table[dest].queue_tail = (tail + 1) % 16;
    port_table[dest].queue_count++;
    
    printf("   [Mach] Msg Sent to Port %d (ID: %d)\n", dest, msg->msgh_id);
    return MACH_MSG_SUCCESS;
}

// Receive Message
int mach_msg_receive(mach_msg_header_t* msg, mach_port_t rcv_name) {
    if (rcv_name <= 0 || rcv_name >= MAX_PORTS || !port_table[rcv_name].active) {
        return MACH_RCV_INVALID_NAME;
    }
    
    // Simple blocking check (simulated spinwait for this test)
    int retries = 0;
    while (port_table[rcv_name].queue_count == 0) {
        if (retries++ > 10) return MACH_MSG_IPC_ERROR; // Timeout
        usleep(100);
    }
    
    // Pop message
    int head = port_table[rcv_name].queue_head;
    mach_msg_base_t* stored = port_table[rcv_name].queue[head];
    
    if (msg->msgh_size < stored->header.msgh_size) {
        // Buffer too small
        return MACH_MSG_IPC_ERROR;
    }
    
    memcpy(msg, stored, stored->header.msgh_size);
    free(stored);
    
    port_table[rcv_name].queue_head = (head + 1) % 16;
    port_table[rcv_name].queue_count--;
    
    printf("   [Mach] Msg Received on Port %d (ID: %d)\n", rcv_name, msg->msgh_id);
    return MACH_MSG_SUCCESS;
}
