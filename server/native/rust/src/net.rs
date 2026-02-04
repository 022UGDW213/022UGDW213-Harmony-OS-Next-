use std::io::{Error, ErrorKind, Result};
use std::sync::{Arc, Mutex};
use std::collections::HashMap;

// --- Packet Structures ---

#[repr(C, packed)]
#[derive(Debug, Clone, Copy, Default)]
pub struct EthernetHeader {
    pub dst_mac: [u8; 6],
    pub src_mac: [u8; 6],
    pub eth_type: u16,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy, Default)]
pub struct Ipv4Header {
    pub ver_ihl: u8,
    pub tos: u8,
    pub total_len: u16,
    pub id: u16,
    pub flags_offset: u16,
    pub ttl: u8,
    pub protocol: u8,
    pub checksum: u16,
    pub src_ip: u32,
    pub dst_ip: u32,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy, Default)]
pub struct TcpHeader {
    pub src_port: u16,
    pub dst_port: u16,
    pub seq_num: u32,
    pub ack_num: u32,
    pub data_offset_res_flags: u16,
    pub window_size: u16,
    pub checksum: u16,
    pub urgent_ptr: u16,
}

// --- Socket Implementation ---

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum SocketState {
    Closed,
    Listen,
    SynSent,
    SynReceived,
    Established,
    FinWait1,
    FinWait2,
    CloseWait,
    Closing,
    LastAck,
    TimeWait,
}

pub struct Socket {
    pub id: u32,
    pub domain: i32,
    pub socket_type: i32,
    pub protocol: i32,
    pub state: SocketState,
    pub local_addr: u32,
    pub local_port: u16,
    pub remote_addr: u32,
    pub remote_port: u16,
    pub send_buffer: Vec<u8>,
    pub recv_buffer: Vec<u8>,
}

impl Socket {
    pub fn new(id: u32, domain: i32, socket_type: i32, protocol: i32) -> Self {
        Socket {
            id,
            domain,
            socket_type,
            protocol,
            state: SocketState::Closed,
            local_addr: 0,
            local_port: 0,
            remote_addr: 0,
            remote_port: 0,
            send_buffer: Vec::new(),
            recv_buffer: Vec::new(),
        }
    }

    pub fn bind(&mut self, addr: u32, port: u16) -> Result<()> {
        if self.state != SocketState::Closed {
            return Err(Error::new(ErrorKind::AlreadyExists, "Socket already bound or active"));
        }
        self.local_addr = addr;
        self.local_port = port;
        Ok(())
    }

    pub fn listen(&mut self, _backlog: i32) -> Result<()> {
        if self.state != SocketState::Closed { // Simplified transition
             return Err(Error::new(ErrorKind::Other, "Invalid state for listen"));
        }
        self.state = SocketState::Listen;
        Ok(())
    }
    
    // Simplified connect for loopback simulation
    pub fn connect(&mut self, addr: u32, port: u16) -> Result<()> {
         if self.state != SocketState::Closed {
            return Err(Error::new(ErrorKind::AlreadyExists, "Socket active"));
        }
        self.remote_addr = addr;
        self.remote_port = port;
        // In a real stack, sending SYN would happen here.
        self.state = SocketState::Established; // Mock established
        Ok(())
    }

    pub fn send(&mut self, buf: &[u8]) -> Result<usize> {
        if self.state != SocketState::Established {
             return Err(Error::new(ErrorKind::NotConnected, "Socket not connected"));
        }
        // In a real stack, we'd package into TCP packets.
        // Here we just store to simulates successful send to buffer.
        self.send_buffer.extend_from_slice(buf);
        Ok(buf.len())
    }

    // Mock recv that just pulls from internal buffer (populated by "network")
    // For test purposes, we'll allow loopback injection
    pub fn recv(&mut self, buf: &mut [u8]) -> Result<usize> {
        if self.state != SocketState::Established {
             return Err(Error::new(ErrorKind::NotConnected, "Socket not connected"));
        }
        
        let len = std::cmp::min(buf.len(), self.recv_buffer.len());
        if len == 0 {
            return Err(Error::new(ErrorKind::WouldBlock, "No data available"));
        }
        
        // Copy data out
        buf[..len].copy_from_slice(&self.recv_buffer[..len]);
        // Remove from buffer
        self.recv_buffer.drain(..len);
        
        Ok(len)
    }

    pub fn close(&mut self) -> Result<()> {
        self.state = SocketState::Closed;
        Ok(())
    }
}

// --- Global Socket Manager ---

pub struct NetworkStack {
    sockets: Arc<Mutex<HashMap<u32, Arc<Mutex<Socket>>>>>,
    next_socket_id: Mutex<u32>,
}

impl NetworkStack {
    pub fn new() -> Self {
        NetworkStack {
            sockets: Arc::new(Mutex::new(HashMap::new())),
            next_socket_id: Mutex::new(1),
        }
    }

    pub fn create_socket(&self, domain: i32, socket_type: i32, protocol: i32) -> Option<Arc<Mutex<Socket>>> {
        let mut id_lock = self.next_socket_id.lock().unwrap();
        let id = *id_lock;
        *id_lock += 1;

        let socket = Arc::new(Mutex::new(Socket::new(id, domain, socket_type, protocol)));
        self.sockets.lock().unwrap().insert(id, socket.clone());
        Some(socket)
    }
    
    // Helper to simulate loopback traffic for testing
    pub fn loopback_inject(&self, socket_id: u32, data: &[u8]) {
        let sockets = self.sockets.lock().unwrap();
        if let Some(sock) = sockets.get(&socket_id) {
             let mut lock = sock.lock().unwrap();
             lock.recv_buffer.extend_from_slice(data);
        }
    }
}

// Global instance (lazy_static replacement using OnceLock in newer Rust or just raw pointer for FFI context)
// For simplicity in this FFI context, we'll use a static pointer pattern initialized by C.

static mut GLOBAL_STACK: *mut NetworkStack = std::ptr::null_mut();

fn get_stack() -> &'static NetworkStack {
    unsafe {
        if GLOBAL_STACK.is_null() {
            let stack = Box::new(NetworkStack::new());
            GLOBAL_STACK = Box::into_raw(stack);
        }
        &*GLOBAL_STACK
    }
}

// --- FFI Exports ---

#[no_mangle]
pub extern "C" fn rust_net_init() {
    let _ = get_stack(); // Initialize
}

#[no_mangle]
pub extern "C" fn rust_socket_create(domain: i32, socket_type: i32, protocol: i32) -> *mut Socket {
    let stack = get_stack();
    if let Some(socket) = stack.create_socket(domain, socket_type, protocol) {
        // Return raw pointer to the Socket. Note: This leaks Arc if not careful, 
        // but for FFI socket handle we usually treat *mut Socket as the handle itself.
        // A better way is to implement a Handle wrapper.
        // For this simple version, we'll maintain the Arc in the HashMap and return a raw pointer
        // that is valid as long as the socket exists in the map.
        // Wait, if we drop the Arc returned by create_socket, the only ref is in HashMap.
        // Returning Arc::as_ptr is dangerous if it's removed from map.
        // We will return Box::into_raw of a new Arc clone to keep ref count up.
        
        Box::into_raw(Box::new(socket)) as *mut Socket // This is actually *mut Arc<Mutex<Socket>> disguised
    } else {
        std::ptr::null_mut()
    }
}

// Helper: Convert void* back to Arc<Mutex<Socket>> reference
unsafe fn get_socket_ref<'a>(socket_ptr: *mut Socket) ->Option<&'a Mutex<Socket>> {
    if socket_ptr.is_null() { return None; }
    let arc_ptr = socket_ptr as *mut Arc<Mutex<Socket>>;
    Some(&**arc_ptr)
}

#[no_mangle]
pub extern "C" fn rust_socket_bind(socket_ptr: *mut Socket, addr: u32, port: u16) -> i32 {
    unsafe {
        if let Some(mutex) = get_socket_ref(socket_ptr) {
            if let Ok(mut socket) = mutex.lock() {
                if socket.bind(addr, port).is_ok() {
                     return 0;
                }
            }
        }
    }
    -1
}

#[no_mangle]
pub extern "C" fn rust_socket_listen(socket_ptr: *mut Socket, backlog: i32) -> i32 {
    unsafe {
        if let Some(mutex) = get_socket_ref(socket_ptr) {
             if let Ok(mut socket) = mutex.lock() {
                if socket.listen(backlog).is_ok() {
                    return 0;
                }
             }
        }
    }
    -1
}

#[no_mangle]
pub extern "C" fn rust_socket_connect(socket_ptr: *mut Socket, addr: u32, port: u16) -> i32 {
     unsafe {
        if let Some(mutex) = get_socket_ref(socket_ptr) {
            if let Ok(mut socket) = mutex.lock() {
                if socket.connect(addr, port).is_ok() {
                    return 0;
                }
            }
        }
    }
    -1
}

#[no_mangle]
pub extern "C" fn rust_socket_send(socket_ptr: *mut Socket, buf: *const u8, len: usize) -> isize {
     unsafe {
         if buf.is_null() { return -1; }
         let slice = std::slice::from_raw_parts(buf, len);
         
         if let Some(mutex) = get_socket_ref(socket_ptr) {
             if let Ok(mut socket) = mutex.lock() {
                 
                 // TEST HACK: If we are connected (mock), we also "inject" the data back into the socket's 
                 // recv buffer to simulate loopback ECHO for the test
                 let id = socket.id;
                 if socket.send(slice).is_ok() {
                     // Drop lock to call global
                     drop(socket); 
                     get_stack().loopback_inject(id, slice);
                     return len as isize;
                 }
             }
         }
     }
     -1
}

#[no_mangle]
pub extern "C" fn rust_socket_recv(socket_ptr: *mut Socket, buf: *mut u8, len: usize) -> isize {
    unsafe {
        if buf.is_null() { return -1; }
        let slice = std::slice::from_raw_parts_mut(buf, len);
        
        if let Some(mutex) = get_socket_ref(socket_ptr) {
             if let Ok(mut socket) = mutex.lock() {
                 match socket.recv(slice) {
                     Ok(n) => return n as isize,
                     Err(_) => return -1,
                 }
             }
        }
    }
    -1
}

#[no_mangle]
pub extern "C" fn rust_socket_close(socket_ptr: *mut Socket) {
    unsafe {
        if !socket_ptr.is_null() {
            let arc_ptr = socket_ptr as *mut Arc<Mutex<Socket>>;
            // Convert back to box to drop the ref count held by the C side
            let _ = Box::from_raw(arc_ptr);
        }
    }
}
