use std::io::{Error, ErrorKind, Result};
use std::sync::{Arc, Mutex};
use std::collections::HashMap;

// --- VFS Traits ---

pub trait FileSystem: Send + Sync {
    fn open(&self, path: &str, flags: i32) -> Result<Box<dyn FileHandle>>;
    fn mkdir(&self, path: &str) -> Result<()>;
    fn list(&self, path: &str) -> Result<Vec<String>>;
}

pub trait FileHandle: Send + Sync {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize>;
    fn write(&mut self, buf: &[u8]) -> Result<usize>;
    fn close(&mut self) -> Result<()>;
}

// --- RamFS Implementation ---

#[derive(Clone)]
struct RamFile {
    name: String,
    data: Arc<Mutex<Vec<u8>>>,
}

struct RamDirectory {
    name: String,
    files: Mutex<HashMap<String, RamFile>>,
    subdirs: Mutex<HashMap<String, Arc<RamDirectory>>>,
}

pub struct RamFileSystem {
    root: Arc<RamDirectory>,
}

impl RamFileSystem {
    pub fn new() -> Self {
        RamFileSystem {
            root: Arc::new(RamDirectory {
                name: "/".to_string(),
                files: Mutex::new(HashMap::new()),
                subdirs: Mutex::new(HashMap::new()),
            }),
        }
    }
}

// Simple handle for an open file
struct RamFileHandle {
    file: Arc<Mutex<Vec<u8>>>,
    position: usize,
}

impl FileHandle for RamFileHandle {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        let data = self.file.lock().unwrap();
        if self.position >= data.len() {
            return Ok(0);
        }
        
        let len = std::cmp::min(buf.len(), data.len() - self.position);
        buf[..len].copy_from_slice(&data[self.position..self.position + len]);
        self.position += len;
        Ok(len)
    }

    fn write(&mut self, buf: &[u8]) -> Result<usize> {
        let mut data = self.file.lock().unwrap();
        // Simple append/overwrite at position
        if self.position + buf.len() > data.len() {
            data.resize(self.position + buf.len(), 0);
        }
        
        data[self.position..self.position + buf.len()].copy_from_slice(buf);
        self.position += buf.len();
        Ok(buf.len())
    }

    fn close(&mut self) -> Result<()> {
        Ok(())
    }
}

impl FileSystem for RamFileSystem {
    fn open(&self, path: &str, flags: i32) -> Result<Box<dyn FileHandle>> {
        // Simplified path parsing: support root files only for now, e.g. "/test.txt"
        // or just "test.txt"
        let filename = path.trim_start_matches('/');
        
        if filename.contains('/') {
            return Err(Error::new(ErrorKind::Unsupported, "Subdirectories not fully supported in this demo"));
        }

        let mut files = self.root.files.lock().unwrap();
        
        if let Some(file) = files.get(filename) {
            return Ok(Box::new(RamFileHandle {
                file: file.data.clone(),
                position: 0,
            }));
        }

        // If generic CREATE flag bit is set (assuming 1 for O_CREAT simulation)
        if (flags & 1) != 0 {
            let new_file = RamFile {
                name: filename.to_string(),
                data: Arc::new(Mutex::new(Vec::new())),
            };
            files.insert(filename.to_string(), new_file.clone());
            return Ok(Box::new(RamFileHandle {
                file: new_file.data,
                position: 0,
            }));
        }

        Err(Error::new(ErrorKind::NotFound, "File not found"))
    }

    fn mkdir(&self, _path: &str) -> Result<()> {
        Ok(()) // Stub
    }

    fn list(&self, _path: &str) -> Result<Vec<String>> {
        let files = self.root.files.lock().unwrap();
        Ok(files.keys().cloned().collect())
    }
}

// --- Global FS Manager ---

pub struct FsManager {
    fs: Arc<RamFileSystem>,
    handles: Mutex<HashMap<i32, Box<dyn FileHandle>>>,
    next_fd: Mutex<i32>,
}

impl FsManager {
    pub fn new() -> Self {
        FsManager {
            fs: Arc::new(RamFileSystem::new()),
            handles: Mutex::new(HashMap::new()),
            next_fd: Mutex::new(100), // Start FDs at 100 to avoid syscall conflicts
        }
    }

    pub fn open(&self, path: &str, flags: i32) -> Result<i32> {
        let handle = self.fs.open(path, flags)?;
        let mut id_lock = self.next_fd.lock().unwrap();
        let fd = *id_lock;
        *id_lock += 1;
        
        self.handles.lock().unwrap().insert(fd, handle);
        Ok(fd)
    }

    pub fn read(&self, fd: i32, buf: &mut [u8]) -> Result<usize> {
        let mut handles = self.handles.lock().unwrap();
        if let Some(handle) = handles.get_mut(&fd) {
            handle.read(buf)
        } else {
            Err(Error::new(ErrorKind::NotFound, "Bad file descriptor"))
        }
    }

    pub fn write(&self, fd: i32, buf: &[u8]) -> Result<usize> {
        let mut handles = self.handles.lock().unwrap();
        if let Some(handle) = handles.get_mut(&fd) {
            handle.write(buf)
        } else {
            Err(Error::new(ErrorKind::NotFound, "Bad file descriptor"))
        }
    }

    pub fn close(&self, fd: i32) -> Result<()> {
        let mut handles = self.handles.lock().unwrap();
        if handles.remove(&fd).is_some() {
            Ok(())
        } else {
            Err(Error::new(ErrorKind::NotFound, "Bad file descriptor"))
        }
    }
    
    pub fn ls_root(&self) -> Vec<String> {
        self.fs.list("/").unwrap_or_default()
    }
}

static mut GLOBAL_FS: *mut FsManager = std::ptr::null_mut();

fn get_fs() -> &'static FsManager {
    unsafe {
        if GLOBAL_FS.is_null() {
            let fs = Box::new(FsManager::new());
            GLOBAL_FS = Box::into_raw(fs);
        }
        &*GLOBAL_FS
    }
}

// --- FFI Exports ---

#[no_mangle]
pub extern "C" fn rust_fs_init() -> i32 {
    let _ = get_fs();
    0
}

#[no_mangle]
pub extern "C" fn rust_fs_open(path: *const i8, flags: i32) -> i32 {
    unsafe {
        let c_str = std::ffi::CStr::from_ptr(path);
        if let Ok(path_str) = c_str.to_str() {
            match get_fs().open(path_str, flags) {
                Ok(fd) => fd,
                Err(_) => -1,
            }
        } else {
            -1
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_fs_read(fd: i32, buf: *mut u8, len: usize) -> isize {
    unsafe {
        let slice = std::slice::from_raw_parts_mut(buf, len);
        match get_fs().read(fd, slice) {
            Ok(n) => n as isize,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_fs_write(fd: i32, buf: *const u8, len: usize) -> isize {
    unsafe {
        let slice = std::slice::from_raw_parts(buf, len);
        match get_fs().write(fd, slice) {
            Ok(n) => n as isize,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_fs_close(fd: i32) -> i32 {
    match get_fs().close(fd) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn rust_fs_ls(path: *const i8) -> i32 {
    // For demo, just print to stdout
    let list = get_fs().ls_root();
    println!("Directory listing for /:");
    for name in list {
        println!("  {}", name);
    }
    0
}
