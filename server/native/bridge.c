#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "kernel/scheduler.h"

// Simple Bridge: Writes kernel stats to a JSON file for the Node.js API to consume.
// In a real OS, this would be a /proc or /sys filesystem entry.

#define STATS_FILE "/tmp/harmony_kernel_stats.json"

void bridge_update_stats(void) {
    scheduler_stats_t stats = scheduler_get_stats();
    
    FILE* f = fopen(STATS_FILE, "w");
    if (!f) return;
    
    fprintf(f, "{\n");
    fprintf(f, "  \"running\": true,\n");
    fprintf(f, "  \"processes\": %llu,\n", (unsigned long long)stats.total_processes);
    fprintf(f, "  \"active\": %llu,\n", (unsigned long long)stats.running_processes);
    fprintf(f, "  \"switches\": %llu,\n", (unsigned long long)stats.total_switches);
    fprintf(f, "  \"timestamp\": %ld\n", (long)time(NULL));
    fprintf(f, "}\n");
    
    fclose(f);
}

// Initialization
void bridge_init(void) {
    printf("🌉 Kernel Bridge Initialized (Output: %s)\n", STATS_FILE);
    // Initial write
    bridge_update_stats();
}
