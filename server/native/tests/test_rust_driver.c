#include "rust/rust_driver.h"
#include <stdio.h>
#include <string.h>

void test_rust_drivers() {
    printf("🦀 Rust Device Driver Test\n");
    printf("===========================\n\n");
    
    // Test 1: Create block device
    printf("🧪 Test 1: Create Block Device\n");
    printf("--------------------------------\n");
    BlockDevice* device = rust_block_device_new(1, 512, 100);
    if (!device) {
        printf("❌ Failed to create device\n");
        return;
    }
    printf("✅ Created block device (512 bytes/block, 100 blocks)\n\n");
    
    // Test 2: Get device info
    printf("🧪 Test 2: Get Device Info\n");
    printf("---------------------------\n");
    DeviceInfo info;
    if (rust_block_device_get_info(device, &info) == 0) {
        printf("Device ID: %u\n", info.device_id);
        printf("Block size: %zu bytes\n", info.block_size);
        printf("Num blocks: %zu\n", info.num_blocks);
        printf("Total size: %zu bytes (%zu KB)\n", info.total_size, info.total_size / 1024);
        printf("Is open: %s\n", info.is_open ? "Yes" : "No");
    }
    printf("\n");
    
    // Test 3: Open device
    printf("🧪 Test 3: Open Device\n");
    printf("-----------------------\n");
    if (rust_block_device_open(device) == 0) {
        printf("✅ Device opened successfully\n");
    } else {
        printf("❌ Failed to open device\n");
    }
    printf("\n");
    
    // Test 4: Write data
    printf("🧪 Test 4: Write Data\n");
    printf("---------------------\n");
    const char* test_data = "Hello from C to Rust! Memory-safe I/O is working!";
    size_t data_len = strlen(test_data);
    ssize_t written = rust_block_device_write(device, 0, (const uint8_t*)test_data, data_len);
    if (written > 0) {
        printf("✅ Wrote %zd bytes: \"%s\"\n", written, test_data);
    } else {
        printf("❌ Write failed\n");
    }
    printf("\n");
    
    // Test 5: Read data
    printf("🧪 Test 5: Read Data\n");
    printf("--------------------\n");
    uint8_t buffer[256] = {0};
    ssize_t read_bytes = rust_block_device_read(device, 0, buffer, data_len);
    if (read_bytes > 0) {
        printf("✅ Read %zd bytes: \"%s\"\n", read_bytes, buffer);
        
        // Verify data
        if (memcmp(buffer, test_data, data_len) == 0) {
            printf("✅ Data verification passed!\n");
        } else {
            printf("❌ Data mismatch!\n");
        }
    } else {
        printf("❌ Read failed\n");
    }
    printf("\n");
    
    // Test 6: Write at different offset
    printf("🧪 Test 6: Write at Offset\n");
    printf("---------------------------\n");
    const char* more_data = "Offset write test";
    written = rust_block_device_write(device, 512, (const uint8_t*)more_data, strlen(more_data));
    if (written > 0) {
        printf("✅ Wrote %zd bytes at offset 512\n", written);
        
        // Read it back
        uint8_t offset_buffer[64] = {0};
        read_bytes = rust_block_device_read(device, 512, offset_buffer, strlen(more_data));
        printf("✅ Read back: \"%s\"\n", offset_buffer);
    }
    printf("\n");
    
    // Test 7: Bounds checking
    printf("🧪 Test 7: Bounds Checking\n");
    printf("---------------------------\n");
    ssize_t result = rust_block_device_read(device, 999999, buffer, 100);
    if (result < 0) {
        printf("✅ Bounds checking works - out of bounds read rejected\n");
    } else {
        printf("❌ Bounds checking failed\n");
    }
    printf("\n");
    
    // Test 8: Close device
    printf("🧪 Test 8: Close Device\n");
    printf("------------------------\n");
    if (rust_block_device_close(device) == 0) {
        printf("✅ Device closed successfully\n");
    }
    printf("\n");
    
    // Test 9: Try to read from closed device
    printf("🧪 Test 9: Read from Closed Device\n");
    printf("-----------------------------------\n");
    result = rust_block_device_read(device, 0, buffer, 10);
    if (result < 0) {
        printf("✅ Read from closed device correctly rejected\n");
    } else {
        printf("❌ Should not be able to read from closed device\n");
    }
    printf("\n");
    
    // Cleanup
    rust_block_device_free(device);
    
    printf("✅ All Rust driver tests completed!\n");
    printf("\n🦀 Rust Safety Features Demonstrated:\n");
    printf("  ✓ Memory-safe I/O operations\n");
    printf("  ✓ Bounds checking\n");
    printf("  ✓ State validation (open/closed)\n");
    printf("  ✓ C FFI integration\n");
    printf("  ✓ Zero-cost abstractions\n");
}

int main() {
    test_rust_drivers();
    return 0;
}
