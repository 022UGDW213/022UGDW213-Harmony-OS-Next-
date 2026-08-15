#ifndef HARMONY_TASK_H
#define HARMONY_TASK_H

#include <stdint.h>
#include "idt.h"   /* registers_t (frame layout saved by kernel/isr.s) */

#define TASK_NAME_MAX   16
#define TASK_STACK_SIZE 4096   /* per-task kernel stack, kmalloc'd */

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DONE
} task_state_t;

typedef void (*task_entry_t)(void);

// Real Task Control Block. `esp` holds a pointer to the registers_t frame
// exactly as isr_common_stub builds it for a preempted task; the scheduler
// iret's into it via kernel/switch.s. Round-robin through `next`.
typedef struct tcb {
    uint32_t    id;
    char        name[TASK_NAME_MAX];
    task_state_t state;
    uint32_t    esp;           // saved interrupted frame ptr (0 before 1st tick)
    uint8_t*    kernel_stack;  // kmalloc'd stack base (NULL for the boot task)
    uint32_t    stack_size;
    uint32_t    ticks;         // scheduler ticks consumed while running
    uint32_t    created_tick;  // PIT tick at creation
    struct tcb* next;          // round-robin ring
} tcb_t;

// Boot the process manager: the current boot context becomes task 0 ("shell")
// and two real worker tasks ("alpha"/"beta") are created that print their
// name/tick so round-robin alternation is observable in serial output.
void task_init(void);

// Create a READY task with a fresh kmalloc'd kernel stack and an initial
// registers_t frame; returns its pid or -1 on failure.
int  task_create(const char* name, task_entry_t entry);

// IRQ0 hook: advances the real PIT counters, saves the interrupted task's
// frame, picks the next READY task and context-switches via kernel/switch.s.
// Installed by kmain AFTER task_init (alloc -> tasks -> scheduler IRQ0).
void scheduler_tick(registers_t* r);

// Mark the current task DONE and park it (never rescheduled).
void task_exit(void);

// Scheduler/Task introspection (used by the `ps` / `sched` shell commands).
uint32_t task_count(void);
uint32_t task_ready_count(void);
uint32_t task_current_id(void);
const char* task_current_name(void);
uint32_t scheduler_switches(void);
const tcb_t* task_get(uint32_t i);   // NULL past the end

#endif // HARMONY_TASK_H
