# Interrupt Subsystem

## Overview
Implemented a complete x86-64 Interrupt Handling subsystem, comprising a populated IDT, low-level assembly ISR stubs, and a high-level C dispatch manager.

## Implementation

### Core Files
#### [server/native/kernel/interrupts.h](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/kernel/interrupts.h)
- **IDT Layout**: 128-bit `idt_entry_t` structure.
- **Register State**: `registers_t` structure pushed by ISR stubs.
- **API**: `register_interrupt_handler` for driver use.

#### [server/native/kernel/interrupts.c](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/kernel/interrupts.c)
- **Initialization**: `interrupts_init` sets up standard vectors (0-31, 128).
- **Dispatch**: Global handler table simulation.

#### [server/native/asm/interrupts.s](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/asm/interrupts.s)
- **ISR Stubs**: Macros for efficient stub generation (`ISR_NOERRCODE`).
- **Context Saving**: Common stub pushing all 15 GP registers.
- **Stack Alignment**: Ensuring ABI compliance before C calls.

#### [server/native/tests/test_interrupts.c](file:///Users/jjsp/Desktop/Cyber%20Lab%20cve%20Vulnarabilities/022UGDW213-Harmony-OS-Next--main/server/native/tests/test_interrupts.c)
- Validates IDT structure and simulates interrupt flows by calling ISR stubs manually.

## Architecture
```mermaid
graph TD
    cpu[CPU Event] -->|Vector| isr[Assembly ISR Stub]
    isr -->|Push Context| common[Common Stub]
    common -->|Call| c_handler[C Dispatcher]
    c_handler -->|Lookup| table[Handler Table]
    table --> specific[Specific Handler (e.g., Timer)]
```

## Features
- **Separation of Concerns**: Assembly handles hardware context; C handles logic.
- **Completeness**: Handling for Exceptions (0-31), IRQs (32-47), and Syscalls (128).
- **Extensibility**: Drivers can register handlers at runtime.

## Verification
- **Output**:
  ```
  ⏰ Timer Interrupt Triggered! (ISR 32)
  🔥 EXCEPTION: Divide by Zero! (ISR 0)
  ```
- All tests passed successfully.
