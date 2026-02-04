#include <stdio.h>
#include "nt/handle.h"
#include "nt/registry.h"
#include "nt/object.h"

// Declare Executive API (usually in a header like ntdef.h)
HANDLE NtCreateKey(const char* name);
int NtSetValueKey(HANDLE key, const char* val_name, int type, void* data, int size);
int NtQueryValueKey(HANDLE key, const char* val_name, void* buf, int size);
void NtClose(HANDLE handle);

void test_nt_executive() {
    printf("🪟 NT Executive & Registry Test\n");
    printf("=============================\n");
    
    ob_init();
    
    // 1. Create Key
    printf("🧪 Test 1: NtCreateKey\n");
    HANDLE hKey = NtCreateKey("\\Registry\\Machine\\System");
    
    if (hKey != INVALID_HANDLE_VALUE) {
        printf("✅ Key created (Handle %d)\n", hKey);
    } else {
        printf("❌ Key creation failed\n");
        return;
    }
    
    // 2. Set Value
    printf("\n🧪 Test 2: NtSetValueKey\n");
    int bootType = 1;
    if (NtSetValueKey(hKey, "BootType", REG_DWORD, &bootType, sizeof(int)) == 0) {
        printf("✅ Value set successfully\n");
    } else {
        printf("❌ SetValue failed\n");
    }
    
    // 3. Query Value
    printf("\n🧪 Test 3: NtQueryValueKey\n");
    int readBack = 0;
    int size = NtQueryValueKey(hKey, "BootType", &readBack, sizeof(int));
    
    if (size == sizeof(int) && readBack == 1) {
        printf("✅ Value queried: %d\n", readBack);
    } else {
        printf("❌ Query failed or mismatch\n");
    }
    
    // 4. Close
    printf("\n🧪 Test 4: NtClose\n");
    NtClose(hKey);
    printf("✅ Handle closed\n");
    
    printf("\n✅ NT Executive tests completed\n");
}

int main() {
    test_nt_executive();
    return 0;
}
