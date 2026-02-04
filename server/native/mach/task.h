#ifndef HARMONY_MACH_TASK_H
#define HARMONY_MACH_TASK_H

#include "mach/port.h"

// Task type (handle)
typedef mach_port_t task_t;
#define MACH_TASK_NULL 0

// Standard Task Special Ports
#define TASK_KERNEL_PORT    1
#define TASK_HOST_PORT      2
#define TASK_NAME_PORT      3
#define TASK_BOOTSTRAP_PORT 4

// API
task_t mach_task_self(void);
task_t task_create(void);
void task_terminate(task_t task);
void task_suspend(task_t task);
void task_resume(task_t task);

// Info
int task_pid(task_t task);
task_t task_from_pid(int pid);

#endif // HARMONY_MACH_TASK_H
