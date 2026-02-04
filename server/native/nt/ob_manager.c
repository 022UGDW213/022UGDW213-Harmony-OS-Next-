#include "nt/object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified Object Manager
// In real NT, headers are complex and object types have methods (Open, Close, Delete).

void ob_init(void) {
    printf("🟦 NT Object Manager Initialized\n");
}

void* ob_create_object(uint32_t type, size_t body_size, const char* name) {
    // Allocate header + body
    size_t total_size = sizeof(object_header_t) + body_size;
    void* ptr = malloc(total_size);
    if (!ptr) return NULL;
    
    object_header_t* header = (object_header_t*)ptr;
    memset(header, 0, sizeof(object_header_t));
    
    header->type = type;
    header->ref_count = 1; // Start with 1 reference
    header->handle_count = 0;
    if (name) {
        strncpy(header->name, name, OB_NAME_MAX - 1);
    }
    
    // Return pointer to body
    void* body = (char*)ptr + sizeof(object_header_t);
    printf("   [Ob] Created Object '%s' (Type %d) Ref=1\n", name ? name : "Unnamed", type);
    return body;
}

object_header_t* ob_header(void* object) {
    if (!object) return NULL;
    return (object_header_t*)((char*)object - sizeof(object_header_t));
}

void ob_reference_object(void* object) {
    object_header_t* header = ob_header(object);
    if (!header) return;
    
    header->ref_count++;
    // printf("   [Ob] Ref Object '%s' -> %d\n", header->name, header->ref_count);
}

void ob_dereference_object(void* object) {
    object_header_t* header = ob_header(object);
    if (!header) return;
    
    header->ref_count--;
    // printf("   [Ob] Deref Object '%s' -> %d\n", header->name, header->ref_count);
    
    if (header->ref_count <= 0) {
        printf("   [Ob] Destroying Object '%s' (Ref=0)\n", header->name);
        free(header);
    }
}
