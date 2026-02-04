// kernel/hal/common/hal_common.c - Common HAL utilities
#include "../hal.h"
#include <stdarg.h>

// hal_printf and hal_vprintf moved to hal_stdio.c

void* hal_memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

// HAL initialization
void hal_early_init(void) {
    // Set state to initializing
    hal.state = HAL_INITIALIZING;
    
    // Early console (just enough to print errors)
    HAL_PUTS("[HAL] Early initialization...\n");
}

void hal_late_init(void) {
    // Call backend-specific init
    if (hal.init) {
        hal.init();
    }
    
    hal.state = HAL_READY;
    HAL_PRINTF("[HAL] %s backend fully initialized\n", hal.name);
}
