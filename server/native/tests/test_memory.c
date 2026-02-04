#include "kernel/memory.h"
#include <stdio.h>
#include <string.h>

void test_memory_management() {
    printf("💾 Memory Management System Test\n");
    printf("==================================\n\n");
    
    // Initialize with 64 MB of physical memory
    memory_manager_t* mm = memory_init(64 * 1024 * 1024);
    if (!mm) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return;
    }
    
    print_memory_stats(mm);
    
    // Test 1: Physical page allocation
    printf("🧪 Test 1: Physical Page Allocation\n");
    printf("------------------------------------\n");
    void* page1 = alloc_physical_page(mm);
    void* page2 = alloc_physical_page(mm);
    void* page3 = alloc_physical_page(mm);
    
    printf("Allocated 3 physical pages:\n");
    printf("  Page 1: %p\n", page1);
    printf("  Page 2: %p\n", page2);
    printf("  Page 3: %p\n", page3);
    printf("\n");
    
    // Test 2: Virtual memory allocation (Windows VirtualAlloc-style)
    printf("🧪 Test 2: Virtual Memory Allocation\n");
    printf("-------------------------------------\n");
    void* virt1 = alloc_virtual(mm, (void*)0x10000000, 8192, MEM_PROT_RW);
    void* virt2 = alloc_virtual(mm, (void*)0x20000000, 16384, MEM_PROT_RWX);
    
    printf("Allocated virtual memory:\n");
    printf("  Region 1: %p (8 KB, RW)\n", virt1);
    printf("  Region 2: %p (16 KB, RWX)\n", virt2);
    printf("\n");
    
    // Test 3: Reserve and commit (Windows-style)
    printf("🧪 Test 3: Reserve and Commit\n");
    printf("------------------------------\n");
    void* reserved = reserve_virtual(mm, (void*)0x30000000, 65536);
    printf("Reserved 64 KB at %p\n", reserved);
    
    bool committed = commit_virtual(mm, reserved, 4096, MEM_PROT_READ);
    printf("Committed first 4 KB: %s\n", committed ? "✅ Success" : "❌ Failed");
    printf("\n");
    
    // Test 4: Memory protection
    printf("🧪 Test 4: Memory Protection\n");
    printf("-----------------------------\n");
    bool protected = protect_memory(mm, virt1, 8192, MEM_PROT_READ);
    printf("Changed protection to read-only: %s\n", protected ? "✅ Success" : "❌ Failed");
    printf("\n");
    
    // Test 5: Multiple page allocation
    printf("🧪 Test 5: Multiple Page Allocation\n");
    printf("------------------------------------\n");
    void* pages = alloc_physical_pages(mm, 10);
    printf("Allocated 10 contiguous pages starting at %p\n", pages);
    printf("\n");
    
    // Print final statistics
    print_memory_stats(mm);
    
    // Test 6: Free memory
    printf("🧪 Test 6: Free Memory\n");
    printf("----------------------\n");
    free_virtual(mm, virt1, 8192);
    free_virtual(mm, virt2, 16384);
    free_physical_page(mm, page1);
    free_physical_page(mm, page2);
    free_physical_page(mm, page3);
    free_physical_pages(mm, pages, 10);
    
    printf("\n");
    print_memory_stats(mm);
    
    // Cleanup
    memory_shutdown(mm);
    
    printf("✅ All memory management tests completed!\n");
}

int main() {
    test_memory_management();
    return 0;
}
