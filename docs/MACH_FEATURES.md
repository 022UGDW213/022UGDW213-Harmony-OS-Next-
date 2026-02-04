# Darwin/Mach Features Implementation

## Overview
Implemented the **Mach IPC Subsystem**, the foundation of the Darwin kernel (macOS/iOS). This provides message-passing primitives (`ports` and `messages`) as the primary mechanism for inter-process communication, replacing traditional function calls.

## Components

### [Mach Ports](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/mach/port.h)
- **Concept**: A port is a secure, kernel-protected communication channel.
- **Implementation**: Integer handles mapped to kernel objects (`ipc_port`).
- **Rights**: `SEND` and `RECEIVE` rights ensure usage security.

### [Mach Messages](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/mach/message.h)
- **Structure**:
    - `header`: Metadata (destination, reply port, ID).
    - `body`: Types data (currently fixed buffer).
- **Semantics**:
    - `mach_msg_send()`: Enqueues message to destination port.
    - `mach_msg_receive()`: Dequeues message from local port.

### [Mach Tasks](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/mach/task.h)
- **Concept**: A task is a container for threads and resources (address space).
- **Implementation**: Mapped to OS Processes (PID).
- **Control**:
    - `task_create()`: Spawns a new process/task.
    - `task_suspend()`: Suspends execution of the task (Process state -> SUSPENDED).
    - `task_resume()`: Resumes execution (Process state -> READY).
    - `task_terminate()`: Kills the underlying process.

### [BSD Compatibility](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/bsd/bsd_shim.c)
- **Concept**: POSIX personality layer on top of Mach.
- **Signals**:
    - `SIGSTOP` -> `task_suspend()`
    - `SIGCONT` -> `task_resume()`
    - `SIGKILL/TERM` -> `task_terminate()`
- **Implementation**: Shim functions `bsd_kill` resolve PID -> Task Port -> Mach Operation.

## Verification
`test_mach` validates:
1.  **Port Allocation**: Creation of new ports in the namespace. -- ✅ PASS
2.  **Message Flow**: Sending from Port A -> Queue -> Receiving on Port B. -- ✅ PASS
3.  **Data Integrity**: Payload preservation. -- ✅ PASS

`test_mach_task` validates:
1.  **Task Creation**: Spawning via Mach API. -- ✅ PASS
2.  **Lifecycle**: Suspend/Resume/Terminate propagation to Scheduler. -- ✅ PASS

`test_bsd` validates:
1.  **Signals**: Delivery of standard POSIX signals via Mach shim. -- ✅ PASS

