// kernel/alloc.c - REAL dynamic memory allocator for the bare-metal kernel.
// A first-fit free-list allocator over the 8 MiB kernel heap region defined
// in linker.ld directly above the kernel image. No libc, no page tables:
// blocks are carved straight out of physical RAM whose availability is
// validated against the real Multiboot BIOS memory map at boot (see
// meminfo_region_available). kmalloc/kfree are real operations: splitting
// on allocation, address-ordered insertion + forward/backward coalescing
// on free.
#include "alloc.h"
#include "meminfo.h"
#include "kprintf.h"
#include "string.h"

extern uint32_t __heap_start;   // linker.ld: first byte of the heap region
extern uint32_t __heap_end;     // linker.ld: last byte (exclusive)

#define BLOCK_MAGIC_FREE 0xF4EEA11Cu
#define BLOCK_MAGIC_USED 0x8AD0CAFEu

#define ALIGNMENT 8
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block {
    uint32_t magic;        // BLOCK_MAGIC_FREE / BLOCK_MAGIC_USED
    uint32_t size;         // total block size in bytes (header included)
    struct block* next;    // free list only (payload lives here when used)
} block_t;

#define HEADER_SIZE (ALIGN_UP(sizeof(block_t)))  // 16 bytes
#define MIN_BLOCK   (HEADER_SIZE + 8)            // smallest splittable block

static block_t* free_head = NULL;
static uint32_t heap_base_addr = 0;
static uint32_t heap_region_size = 0;
static uint32_t live_allocations = 0;   // live kmalloc calls
static uint32_t used_bytes = 0;         // bytes held by live allocations
static uint32_t total_allocs = 0;       // since boot
static uint32_t total_frees = 0;        // since boot

static block_t* block_from_ptr(void* ptr) {
    return (block_t*)((uint32_t)ptr - HEADER_SIZE);
}

void alloc_init(void) {
    heap_base_addr = (uint32_t)&__heap_start;
    heap_region_size = (uint32_t)&__heap_end - (uint32_t)&__heap_start;

    kprintf("[ALLOC] heap region %x..%x (%u KiB)\n",
            heap_base_addr, heap_base_addr + heap_region_size,
            heap_region_size >> 10);

    if (meminfo_region_available(heap_base_addr, heap_region_size)) {
        kprintf("[ALLOC] region validated against Multiboot mmap: AVAILABLE\n");
    } else {
        kprintf("[ALLOC] WARNING: region not inside an available mmap entry\n");
    }

    // One free block covering the whole region.
    block_t* first = (block_t*)heap_base_addr;
    first->magic = BLOCK_MAGIC_FREE;
    first->size  = heap_region_size;
    first->next  = NULL;
    free_head = first;

    kprintf("[ALLOC] allocator ready: %u bytes free\n", heap_free_bytes());
}

void* kmalloc(uint32_t size) {
    if (size == 0) {
        size = 1;
    }
    uint32_t need = HEADER_SIZE + ALIGN_UP(size);

    block_t** link = &free_head;
    while (*link) {
        block_t* b = *link;
        if (b->magic != BLOCK_MAGIC_FREE) {
            kprintf("[ALLOC] corruption: bad magic %x in free list!\n", b->magic);
            return NULL;
        }
        if (b->size >= need) {
            if (b->size - need >= MIN_BLOCK) {
                // Split: the remainder stays a free block right after us.
                block_t* rest = (block_t*)((uint32_t)b + need);
                rest->magic = BLOCK_MAGIC_FREE;
                rest->size  = b->size - need;
                rest->next  = b->next;
                b->size     = need;
                *link = rest;
            } else {
                *link = b->next;
            }
            b->magic = BLOCK_MAGIC_USED;
            b->next  = NULL;
            used_bytes += b->size;
            live_allocations++;
            total_allocs++;
            return (void*)((uint32_t)b + HEADER_SIZE);
        }
        link = &b->next;
    }
    kprintf("[ALLOC] kmalloc(%u) FAILED - heap exhausted\n", size);
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    block_t* b = block_from_ptr(ptr);
    if (b->magic != BLOCK_MAGIC_USED) {
        kprintf("[ALLOC] kfree: bad magic %x (double free?)\n", b->magic);
        return;
    }
    b->magic = BLOCK_MAGIC_FREE;
    used_bytes -= b->size;
    live_allocations--;
    total_frees++;

    // Address-ordered insertion into the free list.
    block_t** link = &free_head;
    while (*link && (uint32_t)*link < (uint32_t)b) {
        link = &(*link)->next;
    }
    b->next = *link;
    *link = b;

    // Coalesce with the next free block (forward adjacency).
    while (b->next && (uint32_t)b + b->size == (uint32_t)b->next) {
        b->size += b->next->size;
        b->next  = b->next->next;
    }
    // Coalesce with the previous free block (backward adjacency).
    if (free_head != b) {
        block_t* prev = free_head;
        while (prev->next && prev->next != b) {
            prev = prev->next;
        }
        if ((uint32_t)prev + prev->size == (uint32_t)b) {
            prev->size += b->size;
            prev->next  = b->next;
        }
    }
}

uint32_t heap_base(void)        { return heap_base_addr; }
uint32_t heap_size(void)        { return heap_region_size; }
uint32_t heap_used_bytes(void)  { return used_bytes; }
uint32_t heap_alloc_count(void) { return live_allocations; }

uint32_t heap_free_bytes(void) {
    uint32_t free = 0;
    for (block_t* b = free_head; b; b = b->next) {
        free += b->size;
    }
    return free;
}

uint32_t heap_free_blocks(void) {
    uint32_t n = 0;
    for (block_t* b = free_head; b; b = b->next) {
        n++;
    }
    return n;
}
