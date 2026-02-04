#include "scheduler_advisor.h"
#include "qbm.h"
#include <stdio.h>

#define MAX_PIDS 64
#define FEATURES 3
// Features:
// 0: Needs CPU (High P1 = Needs CPU)
// 1: I/O Bound (High P1 = Waiting often)
// 2: Anomaly Score (High P1 = Weird behavior)

static qmatrix_t global_qmatrix;

void advisor_init(void) {
    qbm_init(&global_qmatrix, MAX_PIDS, FEATURES);
    printf("🔮 Quantum Advisor Initialized (64x3 Matrix)\n");
}

void advisor_process_created(int pid) {
    if (pid < MAX_PIDS) {
        qbm_reset_row(&global_qmatrix, pid);
    }
}

void advisor_process_exited(int pid) {
    if (pid < MAX_PIDS) {
        qbm_reset_row(&global_qmatrix, pid);
    }
}

int advisor_get_priority_boost(int pid) {
    if (pid >= MAX_PIDS) return 0;
    
    // Calculate boost based on superposition
    // Feature 0 (Needs CPU): If high, give boost
    qbit_t* qb_cpu = qbm_get(&global_qmatrix, pid, 0);
    
    // Feature 2 (Anomaly): If high, maybe punish? Or flag?
    // For now, let's just use CPU need.
    
    if (qb_cpu && qb_cpu->p1 > 0.6f) {
        // High probability of needing CPU
        // Observe it to see if we actually intervene this time
        if (qbm_observe(&global_qmatrix, pid, 0)) {
            return 2; // Quantum Boost!
        }
    }
    
    return 0;
}

void advisor_report_behavior(int pid, int behavior_type) {
    // 0=USED_FULL_SLICE (CPU Hungry), 1=YIELDED (Nice), 2=BLOCKED (I/O)
    
    if (behavior_type == 0) {
        // Used full slice -> Needs CPU
        qbm_update(&global_qmatrix, pid, 0, 0.2f); // Boost "Needs CPU" p1
    } 
    else if (behavior_type == 1) {
        // Yielded -> Doesn't strictly *need* CPU right now as much
        qbm_update(&global_qmatrix, pid, 0, -0.1f); // Lower "Needs CPU" p1 slightly
    }
}
