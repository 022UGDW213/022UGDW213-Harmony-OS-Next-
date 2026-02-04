#include <stdio.h>
#include "nt/handle.h"
#include "nt/registry.h"

// The Executive layer wraps kernel internals into a public "Nt" API.
// Usually in ntoskrnl.exe

// Wrappers for Files (Mock)
HANDLE NtCreateFile(const char* filename) {
    // In real NT, this takes OBJECT_ATTRIBUTES and creates a File Object
    printf("🪟 [Exec] NtCreateFile(%s)\n", filename);
    // Returning dummy handle
    return (HANDLE)0xBADF00D;
}

// Wrappers for Registry
HANDLE NtCreateKey(const char* name) {
    printf("🪟 [Exec] NtCreateKey(%s)\n", name);
    return nt_create_key(name);
}

int NtSetValueKey(HANDLE key, const char* val_name, int type, void* data, int size) {
    return nt_set_value_key(key, val_name, type, data, size);
}

int NtQueryValueKey(HANDLE key, const char* val_name, void* buf, int size) {
    return nt_query_value_key(key, val_name, buf, size);
}

void NtClose(HANDLE handle) {
    printf("🪟 [Exec] NtClose(%d)\n", handle);
    ob_close_handle(handle);
}
