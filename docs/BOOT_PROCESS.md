# Boot Process & Hardware Initialization

## Overview
Implemented the **OS Kernel Boot Sequence**, unifying all core subsystems (Memory, Syscalls, Scheduler, Interrupts, FS, Net) into a single executable `harmony_kernel`.

## Implementation

### Core Files
#### [server/native/boot/main.c](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/boot/main.c)
- **Kernel Entry**: Orchestrates the strict initialization order.
- **Init Process**: Spawns the first user-space (simulated) process `PID 1`.

#### [server/native/boot/hardware.c](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/boot/hardware.c)
- **Hardware Detection**: Simulates CPUID and E820 memory map detection.

### Boot Sequence
1. **Hardware Detect**: Identify CPU (HarmonyCPU) and Memory (128MB).
2. **Interrupts Init**: Setup Mock IDT.
3. **Memory Init**: Initialize Buddy Allocator & Paging.
4. **Scheduler Init**: Setup Process Table.
5. **Spawn Init**: Create PID 1 with own stack.
6. **Start Multitasking**: `schedule()` switches execution to Init.

## Architecture
```mermaid
graph TD
    entry[Entry Point (main)] -->|1. Detect| hw[Hardware]
    entry -->|2. Setup| idt[Interrupts (IDT)]
    entry -->|3. Init| mm[Memory Manager]
    entry -->|4. Init| sched[Scheduler]
    entry -->|5. Fork| init[Init Process (PID 1)]
    entry -->|6. Yield| loop[Scheduler Loop]
    init -->|User Mode| loop
```

## Verification
- **Output Log**:
  ```
  🌌 HarmonyOS Kernel Booting...
  🔍 Detecting Hardware...
  🔧 Initializing IDT...
  💾 Memory Manager initialized
  ⚙️  Scheduler initialized
  🚀 Init Process (PID 1) Started
     ✅ RamFS mounted
     ✅ lo0 up
  ```
- **Result**: ✅ Kernel successfully boots and runs code in a separate process context.
