#ifndef HARMONY_NT_OBJECT_H
#define HARMONY_NT_OBJECT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Object Constants
#define OB_NAME_MAX 64

// Object Header (hidden before the object body)
typedef struct {
    uint32_t type;
    int32_t ref_count;
    int32_t handle_count;
    char name[OB_NAME_MAX];
} object_header_t;

// Object Manager API
void ob_init(void);

// Object Lifecycle
void* ob_create_object(uint32_t type, size_t body_size, const char* name);
void ob_reference_object(void* object);
void ob_dereference_object(void* object);

// Helper to get header from body pointer
object_header_t* ob_header(void* object);

#endif // HARMONY_NT_OBJECT_H
