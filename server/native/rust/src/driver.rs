use std::io::{Error, ErrorKind, Result};
use std::sync::{Arc, Mutex};

/// Device driver trait - all drivers must implement this
pub trait DeviceDriver {
    fn open(&mut self) -> Result<()>;
    fn close(&mut self) -> Result<()>;
    fn read(&mut self, offset: usize, buffer: &mut [u8]) -> Result<usize>;
    fn write(&mut self, offset: usize, data: &[u8]) -> Result<usize>;
    fn ioctl(&mut self, command: u32, arg: usize) -> Result<usize>;
}

/// Generic block device driver
pub struct BlockDevice {
    device_id: u32,
    block_size: usize,
    num_blocks: usize,
    buffer: Vec<u8>,
    is_open: bool,
}

impl BlockDevice {
    pub fn new(id: u32, block_size: usize, num_blocks: usize) -> Result<Self> {
        let total_size = block_size * num_blocks;
        
        Ok(BlockDevice {
            device_id: id,
            block_size,
            num_blocks,
            buffer: vec![0u8; total_size],
            is_open: false,
        })
    }
    
    pub fn get_info(&self) -> DeviceInfo {
        DeviceInfo {
            device_id: self.device_id,
            block_size: self.block_size,
            num_blocks: self.num_blocks,
            total_size: self.block_size * self.num_blocks,
            is_open: self.is_open,
        }
    }
}

impl DeviceDriver for BlockDevice {
    fn open(&mut self) -> Result<()> {
        if self.is_open {
            return Err(Error::new(ErrorKind::AlreadyExists, "Device already open"));
        }
        self.is_open = true;
        Ok(())
    }
    
    fn close(&mut self) -> Result<()> {
        if !self.is_open {
            return Err(Error::new(ErrorKind::NotConnected, "Device not open"));
        }
        self.is_open = false;
        Ok(())
    }
    
    fn read(&mut self, offset: usize, buffer: &mut [u8]) -> Result<usize> {
        if !self.is_open {
            return Err(Error::new(ErrorKind::NotConnected, "Device not open"));
        }
        
        if offset >= self.buffer.len() {
            return Err(Error::new(ErrorKind::InvalidInput, "Offset out of bounds"));
        }
        
        let bytes_to_read = std::cmp::min(buffer.len(), self.buffer.len() - offset);
        buffer[..bytes_to_read].copy_from_slice(&self.buffer[offset..offset + bytes_to_read]);
        
        Ok(bytes_to_read)
    }
    
    fn write(&mut self, offset: usize, data: &[u8]) -> Result<usize> {
        if !self.is_open {
            return Err(Error::new(ErrorKind::NotConnected, "Device not open"));
        }
        
        if offset >= self.buffer.len() {
            return Err(Error::new(ErrorKind::InvalidInput, "Offset out of bounds"));
        }
        
        let bytes_to_write = std::cmp::min(data.len(), self.buffer.len() - offset);
        self.buffer[offset..offset + bytes_to_write].copy_from_slice(&data[..bytes_to_write]);
        
        Ok(bytes_to_write)
    }
    
    fn ioctl(&mut self, command: u32, _arg: usize) -> Result<usize> {
        match command {
            0 => Ok(self.block_size),  // GET_BLOCK_SIZE
            1 => Ok(self.num_blocks),  // GET_NUM_BLOCKS
            _ => Err(Error::new(ErrorKind::InvalidInput, "Unknown ioctl command")),
        }
    }
}

/// Device information structure
#[repr(C)]
pub struct DeviceInfo {
    pub device_id: u32,
    pub block_size: usize,
    pub num_blocks: usize,
    pub total_size: usize,
    pub is_open: bool,
}

/// Thread-safe device manager
pub struct DeviceManager {
    devices: Arc<Mutex<Vec<Box<dyn DeviceDriver + Send>>>>,
}

impl DeviceManager {
    pub fn new() -> Self {
        DeviceManager {
            devices: Arc::new(Mutex::new(Vec::new())),
        }
    }
    
    pub fn register_device(&self, device: Box<dyn DeviceDriver + Send>) -> usize {
        let mut devices = self.devices.lock().unwrap();
        devices.push(device);
        devices.len() - 1
    }
    
    pub fn get_device_count(&self) -> usize {
        self.devices.lock().unwrap().len()
    }
}

// C FFI exports
#[no_mangle]
pub extern "C" fn rust_block_device_new(id: u32, block_size: usize, num_blocks: usize) -> *mut BlockDevice {
    match BlockDevice::new(id, block_size, num_blocks) {
        Ok(device) => Box::into_raw(Box::new(device)),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_free(device: *mut BlockDevice) {
    if !device.is_null() {
        unsafe { 
            let _ = Box::from_raw(device);
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_open(device: *mut BlockDevice) -> i32 {
    if device.is_null() {
        return -1;
    }
    
    unsafe {
        match (*device).open() {
            Ok(_) => 0,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_close(device: *mut BlockDevice) -> i32 {
    if device.is_null() {
        return -1;
    }
    
    unsafe {
        match (*device).close() {
            Ok(_) => 0,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_read(
    device: *mut BlockDevice,
    offset: usize,
    buffer: *mut u8,
    size: usize,
) -> isize {
    if device.is_null() || buffer.is_null() {
        return -1;
    }
    
    unsafe {
        let buf_slice = std::slice::from_raw_parts_mut(buffer, size);
        match (*device).read(offset, buf_slice) {
            Ok(bytes) => bytes as isize,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_write(
    device: *mut BlockDevice,
    offset: usize,
    data: *const u8,
    size: usize,
) -> isize {
    if device.is_null() || data.is_null() {
        return -1;
    }
    
    unsafe {
        let data_slice = std::slice::from_raw_parts(data, size);
        match (*device).write(offset, data_slice) {
            Ok(bytes) => bytes as isize,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_block_device_get_info(device: *mut BlockDevice, info: *mut DeviceInfo) -> i32 {
    if device.is_null() || info.is_null() {
        return -1;
    }
    
    unsafe {
        *info = (*device).get_info();
        0
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_block_device_creation() {
        let device = BlockDevice::new(1, 512, 100).unwrap();
        assert_eq!(device.block_size, 512);
        assert_eq!(device.num_blocks, 100);
        assert_eq!(device.buffer.len(), 512 * 100);
    }
    
    #[test]
    fn test_block_device_read_write() {
        let mut device = BlockDevice::new(1, 512, 100).unwrap();
        device.open().unwrap();
        
        let data = b"Hello, Rust!";
        let written = device.write(0, data).unwrap();
        assert_eq!(written, data.len());
        
        let mut buffer = vec![0u8; data.len()];
        let read = device.read(0, &mut buffer).unwrap();
        assert_eq!(read, data.len());
        assert_eq!(&buffer, data);
    }
}
