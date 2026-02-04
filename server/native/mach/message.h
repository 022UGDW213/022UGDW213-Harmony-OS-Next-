#ifndef HARMONY_MACH_MESSAGE_H
#define HARMONY_MACH_MESSAGE_H

#include <stdint.h>
#include <stddef.h>
#include "mach/port.h"

// Message ID type
typedef int mach_msg_id_t;

// Message Header
typedef struct {
    uint32_t        msgh_bits;
    uint32_t        msgh_size;
    mach_port_t     msgh_remote_port; // Destination
    mach_port_t     msgh_local_port;  // Reply port
    mach_msg_id_t   msgh_id;
} mach_msg_header_t;

// Standard Message Body
typedef struct {
    mach_msg_header_t header;
    char              data[1024]; // Simple fixed buffer for now
} mach_msg_base_t;

// Mach Msg Return Codes
// Returns 0 on success
#define MACH_MSG_SUCCESS    0
#define MACH_SEND_INVALID_DEST  -1
#define MACH_RCV_INVALID_NAME   -2
#define MACH_MSG_IPC_ERROR      -3

// Options
#define MACH_SEND_MSG       0x00000001
#define MACH_RCV_MSG        0x00000002

// API
int mach_msg_send(mach_msg_header_t* msg);
int mach_msg_receive(mach_msg_header_t* msg, mach_port_t rcv_name);

#endif // HARMONY_MACH_MESSAGE_H
