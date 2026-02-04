#ifndef HARMONY_SCHEDULER_ADVISOR_H
#define HARMONY_SCHEDULER_ADVISOR_H

// The Advisor sits between the Scheduler (Logic) and QBM (Probabilistic)
// It interprets QBM states to give "Advice" (Priority Boosts)

void advisor_init(void);

// Called when a process is created
void advisor_process_created(int pid);

// Called when a process exits
void advisor_process_exited(int pid);

// Called by scheduler before picking a process
// Returns a boost value (e.g., 0 to 5) based on Quantum state
int advisor_get_priority_boost(int pid);

// Feedback loop: Report process behavior
// type: 0=CPU_INTENSIVE, 1=IO_BLOCK, 2=YIELD
void advisor_report_behavior(int pid, int behavior_type);

#endif
