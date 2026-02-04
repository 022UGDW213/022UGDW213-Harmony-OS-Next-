#ifndef HARMONY_NT_REGISTRY_H
#define HARMONY_NT_REGISTRY_H

#include "nt/handle.h"
#include <stdint.h>

// Registry Types
#define REG_SZ          1
#define REG_DWORD       4

// Key Object Type ID (Arbitrary unique)
#define OB_TYPE_KEY     100

// API
HANDLE nt_create_key(const char* name);
int nt_set_value_key(HANDLE key, const char* value_name, int type, void* data, int size);
int nt_query_value_key(HANDLE key, const char* value_name, void* buffer, int size);

#endif // HARMONY_NT_REGISTRY_H
