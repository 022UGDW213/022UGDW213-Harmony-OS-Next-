#ifndef HARMONY_ACCESS_CONTROL_H
#define HARMONY_ACCESS_CONTROL_H

#include "security.h"
#include "../kernel/scheduler.h"

bool security_check_capability(process_t* proc, uint32_t cap);
bool security_can_open_file(process_t* proc, const char* path, int mode);
bool security_can_kill(process_t* target, process_t* caller);

#endif
