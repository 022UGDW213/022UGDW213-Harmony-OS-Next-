#ifndef HARMONY_IPC_H
#define HARMONY_IPC_H

#include <stdint.h>

// REAL inter-process communication: fixed-size ring-buffer mailboxes.
// Each slot stores a length byte + payload (max IPC_MSG_MAX-1 bytes).
// ipc_send/ipc_recv are non-blocking primitives; the producer/consumer
// demo tasks poll them ("blocking-ish via polling") and the shell `ipc`
// command prints the real counters.

#define IPC_NAME_MAX   12
#define IPC_MSG_MAX    32      // slot size: 1 length byte + payload
#define IPC_DEPTH      8       // ring capacity in messages
#define IPC_MAX_MAILBOXES 8

typedef struct {
    char     name[IPC_NAME_MAX];
    uint8_t  buf[IPC_DEPTH][IPC_MSG_MAX];   // real ring buffer
    uint32_t head, tail, count;             // ring indices / occupancy
    uint32_t sent_total;                    // messages enqueued (since boot)
    uint32_t recv_total;                    // messages dequeued
    uint32_t dropped_total;                 // send attempts on a full queue
    uint32_t created_tick;                  // PIT tick at creation
} ipc_mailbox_t;

// Create a named mailbox (static array, returns NULL when full).
ipc_mailbox_t* ipc_create(const char* name);

// Enqueue `len` bytes; returns 1 on success, 0 when the queue is full
// (increments dropped_total). Never blocks by itself.
int ipc_send(ipc_mailbox_t* mb, const void* data, uint32_t len);

// Dequeue into `out` (optional) and report the length via `len` (optional);
// returns 1 when a message was available, else 0.
int ipc_recv(ipc_mailbox_t* mb, void* out, uint32_t* len);

// Boot the IPC layer: create the "demo" mailbox and the real
// producer/consumer demo task pair.
void ipc_init(void);

// Introspection for the `ipc` shell command.
uint32_t ipc_count(void);
const ipc_mailbox_t* ipc_get(uint32_t i);   // NULL past the end

#endif // HARMONY_IPC_H
