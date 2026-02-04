#ifndef HARMONY_RUST_DRIVER_H
#define HARMONY_RUST_DRIVER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer to Rust BlockDevice
typedef struct BlockDevice BlockDevice;

// Device information (matches Rust struct)
typedef struct {
    uint32_t device_id;
    size_t block_size;
    size_t num_blocks;
    size_t total_size;
    int is_open;
} DeviceInfo;

// Device creation and destruction
BlockDevice* rust_block_device_new(uint32_t id, size_t block_size, size_t num_blocks);
void rust_block_device_free(BlockDevice* device);

// Device operations
int rust_block_device_open(BlockDevice* device);
int rust_block_device_close(BlockDevice* device);
ssize_t rust_block_device_read(BlockDevice* device, size_t offset, uint8_t* buffer, size_t size);
ssize_t rust_block_device_write(BlockDevice* device, size_t offset, const uint8_t* data, size_t size);
int rust_block_device_get_info(BlockDevice* device, DeviceInfo* info);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_RUST_DRIVER_H
