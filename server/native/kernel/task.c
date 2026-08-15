// kernel/task.c - REAL process manager + round-robin scheduler.
//
// Task Control Blocks with kmalloc'd kernel stacks, and an IRQ0-driven
// context switch: every PIT tick, scheduler_tick() saves the interrupted
// registers_t frame (built by kernel/isr.s on top of the task's own stack)
// into the current TCB, walks the round-robin ring to the next READY task
// and iret's into its saved frame via context_switch (kernel/switch.s).
//
// Three real kernel tasks run: the boot shell (task 0, stack from boot.S)
// plus two worker tasks ("alpha"/"beta") whose name/tick prints make the
// round-robin alternation directly observable in the serial output.
#include "task.h"
#include "alloc.h"
#include "kprintf.h"
#include "string.h"
#include "../drivers/pic.h"
#include "../drivers/pit.h"

#define MAX_TASKS 16

extern void context_switch(uint32_t new_esp);   /* kernel/switch.s */

static tcb_t tasks[MAX_TASKS];
static uint32_t task_total = 0;
static uint32_t next_task_id = 1;
static tcb_t* current = NULL;
static uint32_t ready_count = 0;
static uint32_t switch_count = 0;
static int initialized = 0;

static void set_name(char* dst, uint32_t cap, const char* src) {
    uint32_t i = 0;
    while (src[i] && i < cap - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// ── real worker tasks: print name/tick so alternation is observable ───────
static void task_demo_loop(const char* name, uint32_t stride_ticks) {
    uint32_t last = 0;
    for (;;) {
        uint32_t now = (uint32_t)pit_get_ticks();
        if (now - last >= stride_ticks) {
            last = now;
            kprintf("[TASK] %s alive @ tick=%u (switches=%u)\n",
                    name, now, scheduler_switches());
        }
        // Short poll so we don't hammer the CPU; the scheduler preempts us
        // every 10 ms tick regardless.
        uint32_t until = (uint32_t)pit_get_ticks() + 1;
        while ((uint32_t)pit_get_ticks() < until) { /* poll */ }
    }
}

static void task_demo_alpha(void) { task_demo_loop("alpha", 20); }
static void task_demo_beta(void)  { task_demo_loop("beta", 20); }

void task_init(void) {
    if (initialized) {
        return;
    }
    initialized = 1;
    uint32_t boot_tick = (uint32_t)pit_get_ticks();

    // Task 0 = the current boot context (the shell). Its esp is saved on
    // the first preemption; it keeps the boot.S stack.
    tcb_t* t0 = &tasks[0];
    memset(t0, 0, sizeof(tcb_t));
    t0->id = 0;
    set_name(t0->name, TASK_NAME_MAX, "shell");
    t0->state = TASK_RUNNING;
    t0->esp = 0;
    t0->kernel_stack = NULL;
    t0->stack_size = 0;
    t0->created_tick = boot_tick;
    current = t0;
    t0->next = t0;              // ring: shell -> alpha -> beta -> shell
    task_total = 1;
    ready_count = 1;

    int id_a = task_create("alpha", task_demo_alpha);
    int id_b = task_create("beta", task_demo_beta);
    kprintf("[TASK] init: task0=%s, created alpha=%d beta=%d (ready=%u)\n",
            t0->name, id_a, id_b, ready_count);
}

int task_create(const char* name, task_entry_t entry) {
    if (!initialized || task_total >= MAX_TASKS) {
        return -1;
    }
    uint8_t* stack = (uint8_t*)kmalloc(TASK_STACK_SIZE);
    if (!stack) {
        kprintf("[TASK] create '%s': kmalloc(%u) failed\n", name, TASK_STACK_SIZE);
        return -1;
    }

    tcb_t* t = &tasks[task_total];
    memset(t, 0, sizeof(tcb_t));
    t->id = next_task_id++;
    set_name(t->name, TASK_NAME_MAX, name);
    t->state = TASK_READY;
    t->kernel_stack = stack;
    t->stack_size = TASK_STACK_SIZE;
    t->created_tick = (uint32_t)pit_get_ticks();

    // Craft a registers_t frame on top of the fresh stack, identical to the
    // one isr_common_stub builds for a preempted task. iret resumes the task
    // "as if interrupted" before its first instruction. The frame base is
    // placed so the post-iret esp matches the ABI call entry (esp % 16 == 12).
    uint32_t top = ((uint32_t)stack + TASK_STACK_SIZE) & ~0xFu;
    uint32_t frame_addr = (top - sizeof(registers_t)) & ~0xFu;
    frame_addr -= 8;                       // post-iret esp == 12 (mod 16)
    registers_t* frame = (registers_t*)frame_addr;
    memset(frame, 0, sizeof(registers_t));
    frame->gs = frame->fs = frame->es = frame->ds = 0x10;  // GDT_KERNEL_DATA
    frame->eip    = (uint32_t)entry;
    frame->cs     = 0x08;                  // GDT_KERNEL_CODE
    frame->eflags = 0x202;                 // IF=1, reserved bit 1
    t->esp = (uint32_t)frame;

    // Insert at the tail of the round-robin ring (just before current).
    t->next = current->next;
    current->next = t;

    task_total++;
    ready_count++;
    return (int)t->id;
}

void task_exit(void) {
    if (current && current->state != TASK_DONE) {
        kprintf("[TASK] %s exiting (pid %u)\n", current->name, current->id);
        current->state = TASK_DONE;
        if (ready_count > 0) {
            ready_count--;
        }
    }
    // Park forever; the scheduler skips DONE tasks, the tick ISR keeps
    // waking us so interrupts stay serviced.
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// IRQ0 hook installed by kmain after task_init. Keeps the real PIT counters
// alive, then performs one round-robin context switch per tick.
void scheduler_tick(registers_t* r) {
    pit_tick(r);                       // real 100 Hz tick/uptime counters

    if (!initialized || ready_count <= 1) {
        return;                        // isr_handler sends the EOI as usual
    }

    tcb_t* prev = current;
    prev->esp = (uint32_t)r;           // save the interrupted frame (isr.s)
    prev->ticks++;
    if (prev->state == TASK_RUNNING || prev->state == TASK_READY) {
        prev->state = TASK_READY;
    }                                  // DONE/BLOCKED stay as they are

    // Round-robin: advance to the next READY task in the ring.
    tcb_t* next = prev->next;
    uint32_t guard = 0;
    while (next->state != TASK_READY && guard++ < MAX_TASKS) {
        next = next->next;
    }
    if (next == prev || next->state != TASK_READY) {
        prev->state = TASK_RUNNING;    // nothing else schedulable
        return;
    }

    current = next;
    current->state = TASK_RUNNING;
    switch_count++;

    // EOI must go out now: this handler iret's into the next task and never
    // returns, so isr_handler's own ack (after the hook) would never run and
    // IRQ0 would stay blocked in-service forever.
    pic_ack(0);

    context_switch(current->esp);      // never returns
}

uint32_t task_count(void)         { return task_total; }
uint32_t task_ready_count(void)   { return ready_count; }
uint32_t task_current_id(void)    { return current ? current->id : 0; }
const char* task_current_name(void) {
    return current ? current->name : "none";
}
uint32_t scheduler_switches(void) { return switch_count; }

const tcb_t* task_get(uint32_t i) {
    return (i < task_total) ? &tasks[i] : NULL;
}
