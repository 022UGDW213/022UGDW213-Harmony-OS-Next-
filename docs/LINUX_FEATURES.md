# Linux Features Implementation

## Overview
Implemented **Linux Namespaces**, the foundation of containerization (Docker, LXC). We focused on the **PID Namespace**, allowing process isolation where a process perceives itself as PID 1 (Init) inside a container.

## Components

### [Namespaces](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/linux/namespace.c)
- **Concept**: Resource virtualization.
- **PID Namespace**: Virtualizes Process IDs.
- **Usage**:
    - `create_pid_namespace()`: Creates a new scope.
    - `alloc_pid(ns)`: Allocates a local PID within that scope.

### Integration
- **Scheduler**: Updated `process_t` to hold an `nsproxy` pointer, linking processes to their namespaces.

## Verification
`test_linux` validates:
1.  **Container Creation**: Successfully created a nested PID namespace. -- ✅ PASS
2.  **PID Mapping**: Global PID 2 mapped to Local PID 1 in new namespace. -- ✅ PASS
3.  **Isolation**: Multiple processes in namespace get sequential local PIDs. -- ✅ PASS

## Next Steps (Linux Support)
- **Cgroups**: Resource usage limiting (CPU/RAM).
- **Mount Namespace**: Private filesystem views.
