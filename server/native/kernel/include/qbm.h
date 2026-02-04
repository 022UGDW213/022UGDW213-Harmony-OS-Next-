#ifndef HARMONY_QBM_H
#define HARMONY_QBM_H

#include <stdint.h>
#include "hal/hal.h"

typedef struct {
    int boost_pid;
    float boost_factor;
} qbm_advice_t;

// Stubs for Quantum Brain Matrix
static inline void qbm_init(void) {
    hal_printf("QBM: Initialized (Stub)\n");
}

static inline void qbm_add_process(int pid) {
    (void)pid;
}

static inline void qbm_remove_process(int pid) {
    (void)pid;
}

static inline qbm_advice_t qbm_get_advice(void) {
    qbm_advice_t advice = { -1, 0.0f };
    return advice;
}

static inline void qbm_update_runtime(int pid, uint64_t ticks) {
    (void)pid; (void)ticks;
}

#endif
