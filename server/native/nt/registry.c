#include "nt/registry.h"
#include "nt/object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified Registry Key Object
// In real NT, keys maintain lists of subkeys and values.
// We implement a simple Value Store here.

#define MAX_VALUES 16

typedef struct {
    char name[32];
    int type;
    char data[64]; // Fixed size value storage
    int size;
    bool active;
} reg_value_t;

typedef struct {
    reg_value_t values[MAX_VALUES];
} key_object_t;

HANDLE nt_create_key(const char* name) {
    if (!name) return INVALID_HANDLE_VALUE;
    
    // Allocate Key Object via Object Manager
    key_object_t* key = (key_object_t*)ob_create_object(OB_TYPE_KEY, sizeof(key_object_t), name);
    if (!key) return INVALID_HANDLE_VALUE;
    
    // Initialize values
    memset(key->values, 0, sizeof(key->values));
    
    // Return a Handle to the Key
    return ob_create_handle(key);
}

int nt_set_value_key(HANDLE key_handle, const char* value_name, int type, void* data, int size) {
    key_object_t* key = (key_object_t*)ob_reference_object_by_handle(key_handle);
    if (!key) return -1;
    
    // Check type of object (omitted for simplicity, but ideally check header->type == OB_TYPE_KEY)
    
    // Find free slot or existing value
    for (int i = 0; i < MAX_VALUES; i++) {
        // Simple overwrite if match or create new
        if (!key->values[i].active || strcmp(key->values[i].name, value_name) == 0) {
            strncpy(key->values[i].name, value_name, 31);
            key->values[i].type = type;
            key->values[i].size = (size > 64) ? 64 : size;
            memcpy(key->values[i].data, data, key->values[i].size);
            key->values[i].active = true;
            
            printf("   [Reg] SetValue '%s' = (Type %d, Size %d)\n", value_name, type, size);
            
            ob_dereference_object(key); // Release ref from lookup
            return 0;
        }
    }
    
    ob_dereference_object(key);
    return -1; // Full
}

int nt_query_value_key(HANDLE key_handle, const char* value_name, void* buffer, int size) {
    key_object_t* key = (key_object_t*)ob_reference_object_by_handle(key_handle);
    if (!key) return -1;
    
    for (int i = 0; i < MAX_VALUES; i++) {
        if (key->values[i].active && strcmp(key->values[i].name, value_name) == 0) {
            int copy_size = (key->values[i].size > size) ? size : key->values[i].size;
            memcpy(buffer, key->values[i].data, copy_size);
            
            ob_dereference_object(key);
            return copy_size;
        }
    }
    
    ob_dereference_object(key);
    return -1; // Not found
}
