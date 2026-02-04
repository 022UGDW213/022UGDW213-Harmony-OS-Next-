#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

// Memory statistics structure
typedef struct {
    size_t total_bytes;
    size_t used_bytes;
    size_t free_bytes;
    size_t alloc_count;
    size_t free_count;
} memory_stats_t;

// Initialize memory manager
void memory_init(void);

// HAL-compatible allocation
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t alignment);
void kfree(void* ptr);

// Statistics
void memory_get_stats(memory_stats_t* stats);
void memory_dump_stats(void);

// Validation and testing
int memory_validate(void);
void memory_test(void);

// Early allocation (pre-HAL)
void* memory_early_alloc(size_t size);

#endif // MEMORY_H
