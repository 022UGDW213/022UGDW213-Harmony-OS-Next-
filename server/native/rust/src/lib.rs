pub mod driver;
pub mod net;
pub mod fs;

// Re-export main types
pub use driver::{DeviceDriver, BlockDevice, DeviceManager, DeviceInfo};
