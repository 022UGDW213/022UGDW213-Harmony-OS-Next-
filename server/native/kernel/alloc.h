#ifndef HARMONY_ALLOC_H
#define HARMONY_ALLOC_H

#include <stdint.h>

// Kernel dynamic memory: a real first-fit free-list allocator over the
// fixed kernel heap region [__heap_start, __heap_end) defined in
// linker.ld (8 MiB of free RAM directly above the kernel image). The
// region is validated at boot against the real Multiboot mmap before a
// single byte is handed out.

// Allocate `size` bytes (8-byte aligned); returns NULL when the heap is
// exhausted. Backed by real block splitting.
void* kmalloc(uint32_t size);

// Free a pointer returned by kmalloc; coalesces adjacent free blocks
// address-ordered. Double-free and bad-magic are detected and reported.
void kfree(void* ptr);

// Boot-time setup: locate the region, validate it against the mmap and
// form the single initial free block. Prints real numbers to VGA+serial.
void alloc_init(void);

// Live heap statistics (printed by the `meminfo` shell command).
uint32_t heap_base(void);        // first byte of the heap region
uint32_t heap_size(void);        // region size in bytes
uint32_t heap_used_bytes(void);  // bytes held by live allocations
uint32_t heap_free_bytes(void);  // bytes on the free list
uint32_t heap_alloc_count(void); // live allocations
uint32_t heap_free_blocks(void); // free blocks on the list

#endif // HARMONY_ALLOC_H
