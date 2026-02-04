#include "nt/object.h"
#include "nt/handle.h"
#include <stdio.h>

// Define a test object type
typedef struct {
    int id;
    int data;
} test_object_t;

#define OB_TYPE_TEST 1

void test_nt_kernel() {
    printf("🟦 NT Kernel Test (Object Manager)\n");
    printf("==================================\n");
    
    ob_init();
    
    // 1. Create Object
    printf("🧪 Test 1: Object Creation\n");
    test_object_t* obj = (test_object_t*)ob_create_object(OB_TYPE_TEST, sizeof(test_object_t), "MyTestObject");
    
    if (obj) {
        printf("✅ Object created. RefCount=%d\n", ob_header(obj)->ref_count);
        obj->id = 123;
    } else {
        printf("❌ Object creation failed\n");
        return;
    }
    
    // 2. Handle Management
    printf("\n🧪 Test 2: Handle Creation\n");
    HANDLE h1 = ob_create_handle(obj);
    if (h1 != INVALID_HANDLE_VALUE) {
        printf("✅ Handle %d created. RefCount=%d\n", h1, ob_header(obj)->ref_count);
    } else {
        printf("❌ Handle creation failed\n");
    }
    
    // 3. Reference by Handle
    printf("\n🧪 Test 3: Reference by Handle\n");
    test_object_t* ref_obj = (test_object_t*)ob_reference_object_by_handle(h1);
    if (ref_obj == obj) {
        printf("✅ Lookup successful. RefCount=%d\n", ob_header(obj)->ref_count);
        ob_dereference_object(ref_obj); // Release simple reference
        printf("   Releasing lookup ref. RefCount=%d\n", ob_header(obj)->ref_count);
    } else {
        printf("❌ Lookup failed\n");
    }
    
    // 4. Close Handle
    printf("\n🧪 Test 4: Close Handle\n");
    ob_close_handle(h1);
    printf("   Handle closed. RefCount=%d\n", ob_header(obj)->ref_count);
    
    // 5. Dereference (Destroy)
    printf("\n🧪 Test 5: Final Dereference\n");
    ob_dereference_object(obj); // Should destroy
    
    printf("\n✅ NT Object Manager tests completed\n");
}

int main() {
    test_nt_kernel();
    return 0;
}
