# Harmony Microkernel Architecture

## Overview

The Harmony Microkernel is a lightweight, priority-based process management and inter-process communication (IPC) system inspired by:
- **Harmony OS Zen**: Microkernel design principles
- **Google Chrome**: Multi-process architecture
- **CyberLab**: Distributed intelligence concepts

## Architecture

```
┌──────────────────────────────────────────────────────┐
│              Application Layer                        │
│  (Node.js, React, AI Reasoning Services)             │
└────────────────────┬─────────────────────────────────┘
                     │ Native Bindings (Future)
┌────────────────────▼─────────────────────────────────┐
│          Harmony Microkernel (C++)                    │
│                                                       │
│  ┌─────────────────────────────────────────────┐    │
│  │         Kernel Core                          │    │
│  │  - Process Management                        │    │
│  │  - Priority-Based Scheduling                 │    │
│  │  - Process Control Blocks (PCB)              │    │
│  │  - State Management                          │    │
│  └─────────────────────────────────────────────┘    │
│                                                       │
│  ┌─────────────────────────────────────────────┐    │
│  │         IPC System                           │    │
│  │  - Priority Message Queues                   │    │
│  │  - Thread-Safe Communication                 │    │
│  │  - Broadcast Support                         │    │
│  │  - Timeout Handling                          │    │
│  └─────────────────────────────────────────────┘    │
└───────────────────────────────────────────────────────┘
```

## Components

### 1. Kernel Core (`kernel.h/cpp`)

**Process Control Block (PCB)**:
```cpp
struct ProcessControlBlock {
    int pid;                    // Process ID
    std::string name;           // Process name
    ProcessState state;         // READY, RUNNING, BLOCKED, TERMINATED
    Priority priority;          // CRITICAL, HIGH, NORMAL, LOW
    std::chrono::time_point createdAt;
    std::chrono::time_point lastScheduled;
    size_t memoryUsage;
    void* context;
};
```

**Priority Levels**:
- `CRITICAL (0)` - Highest priority (e.g., GPU process)
- `HIGH (1)` - High priority (e.g., renderer)
- `NORMAL (2)` - Normal priority (e.g., network service)
- `LOW (3)` - Background tasks

**Scheduling Algorithm**:
1. Select all READY processes
2. Sort by priority (lower number = higher priority)
3. For same priority, use round-robin (FIFO)
4. Mark selected process as RUNNING
5. Update last scheduled timestamp

### 2. IPC System (`ipc.h/cpp`)

**Message Structure**:
```cpp
struct Message {
    int id;                     // Unique message ID
    int senderPid;              // Sender process ID
    int receiverPid;            // Receiver process ID
    MessagePriority priority;   // Message priority
    std::string type;           // Message type
    std::string payload;        // Message data
    std::chrono::time_point timestamp;
};
```

**Message Queue**:
- Priority queue (higher priority messages delivered first)
- Thread-safe with mutex and condition variables
- Configurable max size (default: 1000 messages)
- Blocking/non-blocking receive modes
- Timeout support

**IPC Manager**:
- Per-process message queues
- Send/receive operations
- Broadcast to all processes
- Queue lifecycle management

## API Reference

### Kernel API

```cpp
// Create a new process
int createProcess(const std::string& name, Priority priority = Priority::NORMAL);

// Terminate a process
bool terminateProcess(int pid);

// Set process state
bool setProcessState(int pid, ProcessState state);

// Get process info
std::shared_ptr<ProcessControlBlock> getProcess(int pid);

// Schedule next process (returns PID)
int schedule();

// Yield current process
void yield();
```

### IPC API

```cpp
// Create message queue for a process
bool createQueue(int pid, size_t maxSize = 1000);

// Send message
bool sendMessage(int senderPid, int receiverPid, 
                const std::string& type, const std::string& payload,
                MessagePriority priority = MessagePriority::NORMAL);

// Receive message (blocking with timeout)
std::shared_ptr<Message> receiveMessage(int receiverPid, int timeoutMs = -1);

// Broadcast to all processes
void broadcast(int senderPid, const std::string& type, const std::string& payload);
```

## Test Results

### Kernel Tests ✅

**Process Creation**:
- Created 3 processes (WebRenderer, NetworkService, GPUProcess)
- Verified unique PIDs assigned
- Confirmed priority levels set correctly

**Scheduling**:
- CRITICAL priority process (GPUProcess) scheduled consistently
- Priority-based selection working correctly
- Round-robin for same priority verified

**Process Management**:
- Process termination successful
- Process count updated correctly

### IPC Tests ✅

**Priority Message Queue**:
```
Sent Order:        Received Order:
1. LOW             1. CRITICAL (highest)
2. CRITICAL        2. HIGH
3. NORMAL          3. NORMAL
4. HIGH            4. LOW (lowest)
```

**Concurrent Communication**:
- 2 sender threads, 1 receiver thread
- 10 messages total (5 from each sender)
- All messages delivered successfully
- Thread-safe operation verified
- No race conditions or deadlocks

**Broadcast**:
- Message sent to all processes except sender
- Verified delivery to multiple queues

## Performance Characteristics

### Kernel
- **Process creation**: O(1)
- **Scheduling**: O(n log n) where n = number of READY processes
- **Process lookup**: O(log n)

### IPC
- **Send message**: O(log n) where n = queue size
- **Receive message**: O(log n)
- **Thread safety**: Mutex-based locking

## Build Instructions

```bash
cd server/native

# Build everything
make all

# Build specific targets
make brain          # Build brain.cpp
make test_kernel    # Build microkernel tests

# Run tests
make test

# Clean build artifacts
make clean
```

## Integration with Harmony OS Next

### Current Status
- ✅ Microkernel core implemented
- ✅ IPC system implemented
- ✅ Tests passing
- ⏳ Node.js bindings (planned)
- ⏳ Integration with AI reasoning (planned)

### Future Integration

**Node.js Bindings** (N-API):
```javascript
const harmony = require('./native/harmony');

// Create process from Node.js
const pid = harmony.createProcess('AIReasoner', harmony.Priority.HIGH);

// Send IPC message
harmony.sendMessage(1, 2, 'ANALYZE', JSON.stringify(data));

// Receive message
const msg = harmony.receiveMessage(2, 1000);
```

**Use Cases**:
1. **Multi-process AI Reasoning**: Isolate AI providers in separate processes
2. **Security Sandboxing**: Run untrusted code in isolated processes
3. **Resource Management**: Priority-based scheduling for critical tasks
4. **Distributed Intelligence**: IPC for communication between reasoning nodes

## Design Principles

### 1. Minimalism (Zen-inspired)
- Small kernel surface area
- Essential functionality only
- Services in user space

### 2. Priority-Based (Real-time)
- Deterministic scheduling
- Critical tasks always prioritized
- Predictable latency

### 3. Thread-Safe (Chrome-inspired)
- Mutex protection for shared state
- Condition variables for synchronization
- No race conditions

### 4. Extensible
- Clean API boundaries
- Easy to add new features
- Modular design

## Limitations & Future Work

### Current Limitations
- No memory management (uses system malloc)
- No security sandboxing (process isolation only)
- No persistence (in-memory only)
- No distributed capabilities

### Planned Enhancements
1. **Memory Management**: Custom allocators, memory pools
2. **Sandbox**: seccomp-bpf, capability-based security
3. **Persistence**: Process state serialization
4. **Distributed**: Network IPC, cross-device communication
5. **Learning**: Integration with pattern recognition engine

## References

- Harmony OS Zen Architecture
- Google Chrome Multi-Process Architecture
- Microkernel Design Principles
- Real-Time Scheduling Algorithms
