#ifndef HARMONY_NT_HANDLE_H
#define HARMONY_NT_HANDLE_H

#include <stdint.h>

// NT Handle Type
typedef uint32_t HANDLE;

#define INVALID_HANDLE_VALUE ((HANDLE)-1)

// Handle API
HANDLE ob_create_handle(void* object);
void* ob_reference_object_by_handle(HANDLE handle);
void ob_close_handle(HANDLE handle);

#endif // HARMONY_NT_HANDLE_H
