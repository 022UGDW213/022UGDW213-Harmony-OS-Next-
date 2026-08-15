// kernel/ipc.c - REAL fixed-size ring-buffer mailboxes.
//
// Each mailbox is a fixed array of IPC_DEPTH slots; a slot holds a length
// byte followed by the payload. ipc_send/ipc_recv are non-blocking ring
// operations with real counters; the producer/consumer demo task pair uses
// them with bounded polling ("blocking-ish send/recv") and every message is
// observable in the serial output. All counters are real.
#include "ipc.h"
#include "task.h"
#include "kprintf.h"
#include "string.h"
#include "../drivers/pit.h"

static ipc_mailbox_t mailboxes[IPC_MAX_MAILBOXES];
static uint32_t mailbox_count = 0;

static ipc_mailbox_t* demo_mb = NULL;

static void set_name(char* dst, uint32_t cap, const char* src) {
    uint32_t i = 0;
    while (src[i] && i < cap - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

ipc_mailbox_t* ipc_create(const char* name) {
    if (mailbox_count >= IPC_MAX_MAILBOXES) {
        return NULL;
    }
    ipc_mailbox_t* mb = &mailboxes[mailbox_count++];
    set_name(mb->name, IPC_NAME_MAX, name);
    mb->head = mb->tail = mb->count = 0;
    mb->sent_total = mb->recv_total = mb->dropped_total = 0;
    mb->created_tick = (uint32_t)pit_get_ticks();
    return mb;
}

int ipc_send(ipc_mailbox_t* mb, const void* data, uint32_t len) {
    if (!mb || !data) {
        return 0;
    }
    if (len > IPC_MSG_MAX - 1) {
        len = IPC_MSG_MAX - 1;
    }
    if (mb->count >= IPC_DEPTH) {        // queue full: real drop counter
        mb->dropped_total++;
        return 0;
    }
    uint8_t* slot = mb->buf[mb->head];
    slot[0] = (uint8_t)len;
    if (len) {
        memcpy(slot + 1, data, len);
    }
    mb->head = (mb->head + 1) % IPC_DEPTH;
    mb->count++;
    mb->sent_total++;
    return 1;
}

int ipc_recv(ipc_mailbox_t* mb, void* out, uint32_t* len) {
    if (!mb || mb->count == 0) {
        return 0;
    }
    uint8_t* slot = mb->buf[mb->tail];
    uint32_t n = slot[0];
    if (out && n) {
        memcpy(out, slot + 1, n);
    }
    if (len) {
        *len = n;
    }
    mb->tail = (mb->tail + 1) % IPC_DEPTH;
    mb->count--;
    mb->recv_total++;
    return 1;
}

// ── real demo pair: producer fills, consumer drains, both visible ─────────
static void u32_to_dec(char* out, uint32_t v) {
    char tmp[12];
    int i = 0;
    do {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    } while (v);
    while (i) {
        *out++ = tmp[--i];
    }
    *out = '\0';
}

static void ipc_producer(void) {
    uint32_t n = 0;
    uint32_t last = 0;
    char msg[32];
    for (;;) {
        uint32_t now = (uint32_t)pit_get_ticks();
        if (now - last < 15) {           // pace: ~6 messages per second
            continue;
        }
        last = now;

        // Build the payload: "msg N tick T"
        char* p = msg;
        memcpy(p, "msg ", 4);  p += 4;
        u32_to_dec(p, n);      while (*p) p++;
        memcpy(p, " tick ", 6); p += 6;
        u32_to_dec(p, now);    while (*p) p++;
        uint32_t len = (uint32_t)(p - msg);

        // Blocking-ish send via bounded polling; the scheduler preempts us
        // between polls so the consumer gets CPU to drain the queue.
        uint32_t retries = 0;
        while (!ipc_send(demo_mb, msg, len) && retries++ < 500) {
            /* poll for space */
        }
        if (retries >= 500) {
            kprintf("[IPC] prod: queue full, dropped msg %u\n", n);
        } else {
            kprintf("[IPC] prod: sent %u bytes '%s' (total=%u)\n",
                    len, msg, demo_mb->sent_total);
        }
        n++;
    }
}

static void ipc_consumer(void) {
    char buf[IPC_MSG_MAX];
    uint32_t last = 0;
    for (;;) {
        uint32_t now = (uint32_t)pit_get_ticks();
        uint32_t len = 0;
        if (ipc_recv(demo_mb, buf, &len)) {
            buf[len] = '\0';             // kprintf %s needs a terminator
            kprintf("[IPC] cons: got %u bytes '%s' (total=%u @ tick %u)\n",
                    len, buf, demo_mb->recv_total, now);
        } else if (now - last < 5) {     // poll cadence so the producer
            continue;                    // can actually fill the queue
        }
        last = now;
    }
}

void ipc_init(void) {
    demo_mb = ipc_create("demo");
    if (!demo_mb) {
        kprintf("[IPC] failed to create mailbox\n");
        return;
    }
    int p = task_create("producer", ipc_producer);
    int c = task_create("consumer", ipc_consumer);
    kprintf("[IPC] mailbox '%s' depth=%u slot=%u; tasks producer=%d consumer=%d\n",
            demo_mb->name, IPC_DEPTH, IPC_MSG_MAX - 1, p, c);
}

uint32_t ipc_count(void) { return mailbox_count; }

const ipc_mailbox_t* ipc_get(uint32_t i) {
    return (i < mailbox_count) ? &mailboxes[i] : NULL;
}
