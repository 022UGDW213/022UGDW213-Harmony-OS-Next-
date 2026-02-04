#include "nt/handle.h"
#include "nt/object.h"
#include <stdio.h>
#include <stdlib.h>

// Simplified Handle Table
// Real NT uses a multi-level table per process. We use a simple global array for proof of concept.
// In a real integration, this would be part of `process_t`.

#define MAX_HANDLES 1024

typedef struct {
    void* object;
    bool active;
} handle_entry_t;

static handle_entry_t handle_table[MAX_HANDLES];

HANDLE ob_create_handle(void* object) {
    if (!object) return INVALID_HANDLE_VALUE;
    
    // Find free slot
    for (int i = 1; i < MAX_HANDLES; i++) {
        if (!handle_table[i].active) {
            // Reference object because handle exists
            ob_reference_object(object);
            
            handle_table[i].object = object;
            handle_table[i].active = true;
            
            // Update handle count
            ob_header(object)->handle_count++;
            
            printf("   [Handle] Created Handle %d -> Object '%s'\n", i, ob_header(object)->name);
            return (HANDLE)i;
        }
    }
    
    return INVALID_HANDLE_VALUE;
}

void* ob_reference_object_by_handle(HANDLE handle) {
    if (handle <= 0 || handle >= MAX_HANDLES || !handle_table[handle].active) {
        return NULL;
    }
    
    void* obj = handle_table[handle].object;
    ob_reference_object(obj); // Caller gets a reference
    return obj;
}

void ob_close_handle(HANDLE handle) {
    if (handle <= 0 || handle >= MAX_HANDLES || !handle_table[handle].active) {
        printf("   [Handle] Invalid Handle Closure %d\n", handle);
        return;
    }
    
    void* obj = handle_table[handle].object;
    printf("   [Handle] Closing Handle %d -> Object '%s'\n", handle, ob_header(obj)->name);
    
    handle_table[handle].active = false;
    handle_table[handle].object = NULL;
    
    // Decrement handle count and dereference object (handle reference removed)
    ob_header(obj)->handle_count--;
    ob_dereference_object(obj);
}
