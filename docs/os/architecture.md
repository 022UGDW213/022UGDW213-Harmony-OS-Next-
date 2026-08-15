# Harmony OS Next — Hybrid Microkernel + Service-Mesh Architecture

> Status: **living spec** — describes the ACTUAL implementation in
> `server/native/` (verified 2026-08-15, HEAD `8c7fa1a`), plus the hosted
> simulation layers that ship in the tree and the planned bare-metal ports.
> Each subsystem is tagged **REAL** (boots in QEMU, real port/CPU I/O),
> **SIM** (hosted/HAL simulation that compiles and runs on the host, not in
> the freestanding boot), or **PLANNED** (spec-only, no code yet).
> "Hybrid" means exactly this: a genuinely freestanding 32-bit i386 core that
> boots in QEMU, overlaid with the iBot service-mesh control plane
> (34 gateway + 9 Python agents, 43 swarm nodes, 242 skill catalog) as the
> orchestrating layer.

---

## 1. System overview

```
┌────────────────────────────────────────────────────────────────────┐
│ SERVICE MESH (iBot control plane, real, live)                      │
│  gateway :18789 — 34 cluster agents (4 orch + 5 supv + 25 workers) │
│  python  :4003  — 9 FastAPI agents (design/frontend/backend/qa/…)  │
│  ui      :3000  — 43-node swarm graph, 242-skill marketplace        │
│  bridge.c (SIM) — /tmp/harmony_kernel_stats.json → Node.js API      │
├────────────────────────────────────────────────────────────────────┤
│ HYBRID KERNEL (server/native)                                      │
│  REAL  freestanding i386 core: boot.S → kmain_bare.c               │
│        gdt/idt/isr/pic/pit/meminfo/vga/serial/keyboard/shell       │
│  SIM   hosted upper layer: memory/scheduler/syscall/ipc/mach/nt/…   │
│        (HAL abstraction: kernel/hal/{bare,hosted})                  │
│  PLANNED bare-metal ports: paging/allocator/IPC/security/net/fs     │
└────────────────────────────────────────────────────────────────────┘
```

Two build targets share one HAL abstraction (`kernel/hal/hal.h`):

| Target | Build flag | Compiler | Purpose |
|---|---|---|---|
| `harmony_kernel` (bare) | `HARMONY_BARE_METAL` | `gcc -m32 -ffreestanding` | boots in QEMU, real hardware drivers |
| `harmony_hosted` (sim) | `HARMONY_HOSTED` | host `cc` + libc | kernel logic prototype on the host |

---

## 2. Boot chain (REAL)

| Step | File | What really happens |
|---|---|---|
| Multiboot-1 header | `boot/boot.S` | MB1 magic `0x1BADB002`, flags `0x03`, in `.multiboot_header` (first 8 KiB of the LOAD segment so QEMU `-kernel` finds it); `.code32`, `_start`: `cli`, 16 KiB stack (`stack_top`), 16-byte stack align, `push %ebx` (info addr) + `push %eax` (magic), `call kmain` |
| Kernel entry | `kernel/kmain_bare.c` | `kmain(uint32_t magic, uint32_t addr)` — see §3 |
| Alternate entry | `boot/miniboot.S` | 64-bit NASM Multiboot-2 header (`0xe85250d6`) variant, not part of the bare QEMU build |
| MB2 parser | `boot/multiboot.c` | tags: cmdline, bootloader name, basic meminfo, mmap — used on the multiboot2 path |

Boot order in `kmain_bare.c` (lines 21–71):

1. `serial_init()` — COM1 debug channel first.
2. `vga_clear()` + green banner.
3. Magic check — `0x2BADB002` (MB1) or `0x36d76289` (MB2) → `multiboot_parse(addr)`.
4. `gdt_init()` — flat 32-bit descriptors, `lgdt` + far jump (REAL).
5. `pic_remap(32, 40)` — 8259 IRQs to vectors 32–47 (REAL).
6. `idt_init()` — 256 gates from `isr_stub_table` (REAL).
7. `pit_init(100)` + `irq_install_handler(0, pit_tick)` + `pic_mask_irq(0, 0)` (REAL).
8. `meminfo_init(addr)` — real RAM from the MB1 info struct (REAL).
9. `sti` — interrupts live (REAL).
10. `keyboard_init()` + `shell_run()` — interactive shell (REAL).

---

## 3. Kernel core (REAL, bare-metal)

### 3.1 GDT — `kernel/gdt.c` (50 lines)
Three real descriptors: null, flat code `0x08` (`0x9A`, 4 GiB, 4K gran), flat data
`0x10` (`0x92`, 4 GiB). `gdt_flush()` runs `lgdt`, far-jumps to `$0x08`, reloads
`ds/es/fs/gs/ss` with `0x10`. No paging, no TSS, no user segments yet.

### 3.2 IDT + ISR — `kernel/idt.c` + `kernel/isr.s`
- `idt.c`: `idt_entries[256]`, `idt_set_gate()` uses selector `GDT_KERNEL_CODE`,
  int-gate flags `0x8E`; `idtr.limit/base`; `lidt`; `isr_handler(registers_t*)`
  dispatches: IRQ (32–47) → `irq_handlers[irq]` hook + `pic_ack(irq)`; CPU
  exception (0–31) → named kprintf with `err_code`/`eip`; else "Unhandled
  interrupt".
- `isr.s`: macros `ISR_NOERRCODE` / `ISR_ERRCODE` (error-code vectors 8, 10–14,
  16, 17) for vectors 0–47; `isr_common_stub` saves `gs/fs/es/ds` + `pusha`,
  sets kernel segments (`0x10`), calls `isr_handler(registers_t*)` with `esp`,
  restores, `add $8, %esp`, `iret`. `isr_stub_table[48]` in `.data` lets C fill
  the IDT without 48 externs.

### 3.3 PIC — `drivers/pic.c` (61 lines)
8259 real port I/O: ICW1–ICW4 to `0x20/0xA0/0x21/0xA1`, cascade wiring
(master `0x04`, slave `0x02`), 8086 mode, all IRQs masked; `pic_ack()` EOI
(both PICs for IRQ ≥ 8); `pic_mask_irq()` per-IRQ mask.

### 3.4 PIT — `drivers/pit.c` (57 lines)
8253/8254 channel 0, mode 2 rate generator, 100 Hz default (`divisor =
1193182 / hz`), command byte `0x36`; `pit_tick()` (IRQ0 hook) increments
64-bit `pit_ticks` and a seconds counter (32-bit wrap-safe). Exposed to the
shell as `uptime`.

### 3.5 Console + input (REAL)
| Driver | File | Details |
|---|---|---|
| VGA | `drivers/vga.c` | `0xB8000` text buffer, 80×25, `memmove` scroll, `\n \r \b` handling, colors |
| Serial | `drivers/serial.c` | COM1 `0x3F8`, 38400 baud (divisor 3), 8N1, FIFO 14-byte threshold; blocking putc, non-blocking getc (`0x3FD` bit 0) |
| Keyboard | `drivers/keyboard.c` | PS/2 `0x60/0x64`, scancode set 1 map (56 printable keys), non-blocking, break-codes ignored |
| kprintf | `kernel/kprintf.c` | freestanding printf → serial + VGA: `%s %c %d %u %x %p %%`, zero-pad width, 64-bit hex via hi:lo shifts (no 64-bit division in `-nostdlib` mode) |
| string | `kernel/string.c` | memcpy/memmove/memset/strcmp/strlen |

### 3.6 Memory detection — `kernel/meminfo.c` (REAL)
Parses the MB1 info struct: `mem_lower` + `mem_upper` totals and the BIOS
memory map (`mmap_addr/mmap_length`, packed 24-byte entries, type 1 =
available). Tracks total KiB, available MiB, entry count — surfaced by the
shell `meminfo` command.

### 3.7 Shell — `kernel/shell.c` (REAL, interactive)
Prompt `harmony> `; input polled from COM1 RX and PS/2; line editor with
backspace; commands: `help`, `uptime`, `meminfo`, `echo <t>`, `clear`,
`reboot` (8042 reset: `outb 0xFE` → port `0x64`).

---

## 4. Process manager

### Current: hosted simulation (SIM) + one real kernel thread
- `kernel/scheduler.c` (250 lines) — quantum scheduler over HAL:
  `MAX_PROCESSES 64`, `TIME_SLICE_MS 10`, process table, round-robin with
  QBM quantum boost (`quantum_schedule`), `scheduler_tick/yield`, context
  switch is *simulated* in hosted mode (`HARMONY_HOSTED` branch), states
  NEW/READY/RUNNING/BLOCKED/ZOMBIE, per-PID QBM entries.
- `microkernel/kernel.cpp` — C++ `Kernel`: `createProcess/terminateProcess`,
  priority + round-robin `schedule()`, `yield()`, process table.
- `processes/process.cpp` — C++ `Process` base (worker thread + IPC loop),
  `RendererProcess`, `GPUProcess`, `NetworkProcess`, `ProcessManager`.
- `asm/context.c` — x86-64 `context_t` init (rsp/rbp/FPU FCW `0x037F`,
  MXCSR `0x1F80`, entry in `rax`) — the 64-bit hosted variant.
- `usr/init.c` / `usr/shell.c` — hosted PID 1 (init) and user shell (hsh)
  with `ls/ps/cat/whoami/su/dmesg/top/qstat/fpu/cow/exit`.

### Planned (bare-metal, next milestones)
Real 32-bit context switch assembly (like `isr.s`), `scheduler_create_process`
ported onto the bare core with per-process kernel stacks, PIT-tick-driven
preemption (the 100 Hz IRQ0 already runs), then user mode via TSS + ring-3
segments (GDT has room: entries are `gdt_entries[3]` today).

---

## 5. Memory model

| Layer | Status | File | Reality |
|---|---|---|---|
| RAM detection | REAL | `kernel/meminfo.c` | MB1 mem_lower/mem_upper + mmap, real values printed |
| Early bump heap | SIM/PLANNED | `kernel/memory.c` (`memory_early_alloc`) | 4 MiB at `0x100000`, 16-byte aligned; not wired into `kmain_bare.c` |
| HAL allocator | SIM | `kernel/memory.c` | `kmalloc/kfree/kmalloc_aligned` → `HAL_MALLOC/HAL_FREE`; bare backend currently panics on alloc (`kernel/hal/bare/hal.c`) |
| Hosted pool | SIM | `kernel/memory.c` | 1 GiB fake pool when `HARMONY_HOSTED` |
| VM syscalls | SIM | `kernel/syscall.c` | `sys_mmap/sys_munmap/sys_brk` → `alloc_virtual/free_virtual` |
| Paging | PLANNED | — | Page tables, 4 KiB frames from the mmap, COW (shell has a `cow` demo stub) |

Design intent: turn the mmap entries `meminfo_init` already enumerates into a
physical-frame allocator, then a 4 KiB paged address space with
`alloc_virtual` (syscall layer) on top; `kmalloc` then becomes a kernel-heap
front-end.

---

## 6. IPC

| Mechanism | File | Status |
|---|---|---|
| C++ message queue | `microkernel/ipc.cpp` | SIM — per-PID queues, priority queue, mutex+condvar, send/receive(timeout)/broadcast |
| Mach ports | `mach/mach_ipc.c` | SIM — 1024-port table, 16-deep queues, `mach_msg_send/receive` |
| Mach tasks | `mach/mach_task.c` | SIM — port ↔ PID mapping, task_create/terminate/suspend/resume |
| NT object manager | `nt/ob_manager.c` | SIM — header+body objects, refcounts |
| NT handles | `nt/handle_table.c` | SIM — 1024-handle table, reference counting |
| NT registry | `nt/registry.c` + `nt/executive.c` | SIM — key objects, 16 values × 64 B, `NtCreateKey/SetValueKey/QueryValueKey` |
| Host bridge | `bridge.c` | SIM — kernel stats → `/tmp/harmony_kernel_stats.json` for the Node.js API |

Planned bare-metal IPC: kernel message queue (like `microkernel/ipc.cpp`) with
syscall wrappers, copying payloads across the privilege boundary; the Mach/NT
surfaces become compatibility shims on top.

---

## 7. Security sandbox

| Piece | File | Status |
|---|---|---|
| Capability checks | `security/access_control.c` | SIM — UID 0 bypass, `CAP_DAC_OVERRIDE`, `CAP_KILL`, same-UID kill |
| File policy | `security/access_control.c` | SIM — `sensitive.txt` root-only; RamFS otherwise open |
| Syscall enforcement | `kernel/syscall.c` | SIM — `sys_open` denies `EACCES`; `sys_setuid` requires root/`CAP_SETUID`, drops caps when leaving root |
| Process identity | `kernel/include/process.h` | SIM — `process_t.security.{uid,capabilities}` |

Planned: capability bits for each real syscall, per-process sandbox memory
ranges (via paging), and an agent-trust model where the OS sandbox is the
kernel side of the iBot security mesh.

---

## 8. Virtualization / device fabric

- **Hypervisor testbed (REAL)**: QEMU is the target — `-kernel harmony_kernel
  -m 512M -vnc 127.0.0.1:2 -serial file:/tmp/harmony-serial.log`. VNC display
  `:2` → port 5902 → `ws-vnc-bridge` (`127.0.0.1:6080`, path `/novnc2`) →
  noVNC at `http://localhost:3000/novnc/vnc_lite.html?...`.
- **HAL** (`kernel/hal/hal.h`, `kernel/hal/bare/hal.c`, `kernel/hal/hosted/hal.c`):
  the device fabric seam — console/memory/time/interrupt ops as function
  pointers; bare backend wires real drivers, hosted backend wires libc.
- **Drivers (REAL)**: `drivers/{vga,serial,keyboard,pic,pit}.c` — all real
  port I/O (`outb/inb`).
- **Binary loader (SIM)**: `loader/loader.c` detects ELF (`0x7fELF`), PE
  (`MZ`), Mach-O (`0xfeedfacf`) and parses the ELF entry point — the seed for
  an ELF user-space loader.
- **Rust layer (SIM stubs)**: `rust/{rust_fs.h, rust_net.h, rust_driver.h,
  mock_rust.c}` — driver/fs/net API sketches.
- **Linux compat (SIM)**: `linux/namespace.c` — PID namespaces (levels,
  per-namespace pid bump).

Planned: VGA → 80×25 framebuffer modeset + font, PS/2 IRQ-driven keyboard
(IRQ1 is already mapped by the IDT), PIT-driven scheduler clock, then an
8250 UART driver for the planned network stack.

---

## 9. Network stack

- Current: `rust/rust_net.h` + `rust_net_init()` (SIM) — loopback `lo0`
  (`127.0.0.1`) brought up by the hosted init process.
- Planned: real NIC driver on QEMU's e1000 (`-netdev user`), ARP/IPv4/Ethernet
  frame parsing in `kernel/net/`, UDP/TCP sockets as syscalls, and the serial
  console as a character-device fallback. The iBot gateway (`ws://127.0.0.1:
  18789`) is the reference for the service-mesh protocol the OS will eventually
  speak natively.

---

## 10. Filesystem model

- Current: `rust/rust_fs.h` + `rust_fs_init` (SIM) — a RamFS mounted at `/`,
  `/welcome.msg` created by init; `sys_open/read/write/close` currently pass
  through to the host `open/read/write` (`kernel/syscall.c`, SIM).
- Planned: a real in-RAM filesystem backed by the physical allocator (initrd
  from the multiboot `mods` list, or a ramdisk region from the mmap),
  VFS inode API, and `sys_getdents` so the hosted shell's `ls` becomes real.

---

## 11. Agent orchestration layer (service mesh)

- `bridge.c` (SIM) writes live kernel stats to `/tmp/harmony_kernel_stats.json`
  — the /proc-analog the Node.js side consumes.
- The iBot control plane (REAL, live, verified 2026-08-15):
  - Gateway `:18789` — **34** cluster agents (4 orchestrator + 5 supervisor +
    25 worker) over 7 Ollama models (gemma3:12b ×4, llama3.1:8b ×6,
    deepseek-r1:7b ×9, qwen2.5:7b ×5, mistral:7b ×5, gemma3:4b ×1,
    llama3.2:1b ×4).
  - Python agents `:4003` — **9** FastAPI agents (v8.3.0-AGI).
  - UI `:3000` — **43** swarm nodes (34 gateway + 9 python), 15 skill
    categories, **242** skills in the catalog.
  - On-disk skill packs: 685 `SKILL.md` under `.agents/skills/` (superset of
    the 242-skill marketplace catalog).
  - See `cluster-topology.md` for the mapping of this mesh onto OS subsystem
    work.

---

## 12. Subsystem → source map (source of truth)

| Subsystem | File(s) | Status |
|---|---|---|
| Multiboot boot | `boot/boot.S`, `boot/miniboot.S`, `boot/multiboot.c`, `boot/main.c`, `boot/hardware.c` | REAL (MB1 path) |
| GDT | `kernel/gdt.c/h` | REAL |
| IDT + ISR stubs | `kernel/idt.c/h`, `kernel/isr.s` | REAL |
| PIC | `drivers/pic.c/h` | REAL |
| PIT | `drivers/pit.c/h` | REAL |
| VGA | `drivers/vga.c/h` | REAL |
| Serial (COM1) | `drivers/serial.c/h` | REAL |
| Keyboard (PS/2) | `drivers/keyboard.c/h` | REAL |
| kprintf | `kernel/kprintf.c/h` | REAL |
| String lib | `kernel/string.c/h` | REAL |
| RAM detection | `kernel/meminfo.c/h` | REAL |
| Shell | `kernel/shell.c/h` | REAL |
| HAL | `kernel/hal/hal.h`, `kernel/hal/bare/hal.c`, `kernel/hal/hosted/hal.c`, `kernel/hal/common/*`, `kernel/include/hal/config.h` | REAL (bare) / SIM (hosted) |
| Logging | `kernel/log.c/h` | SIM |
| Memory manager | `kernel/memory.c/h` | SIM (HAL) |
| Scheduler | `kernel/scheduler.c/h`, `kernel/include/process.h` | SIM (hosted) |
| Syscalls | `kernel/syscall.c/h` | SIM (hosted passthrough) |
| C++ microkernel | `microkernel/kernel.cpp/h`, `microkernel/ipc.cpp/h` | SIM |
| Processes | `processes/process.cpp/h` | SIM |
| Context switching | `asm/context.c/h` | SIM (x86-64 hosted) |
| Mach IPC | `mach/mach_ipc.c`, `mach/mach_task.c`, `mach/{port,message,task}.h` | SIM |
| NT object/handle/registry | `nt/{ob_manager,handle_table,registry,executive}.c`, `nt/*.h` | SIM |
| Security | `security/access_control.c/h`, `security/security.h` | SIM |
| Loader | `loader/loader.c/h`, `loader/formats.h` | SIM (format detect) |
| Linux compat | `linux/namespace.c/h` | SIM |
| BSD shim | `bsd/bsd_shim.c`, `bsd/signal.h` | SIM |
| Rust layer | `rust/{mock_rust.c, rust_fs.h, rust_net.h, rust_driver.h}` | SIM (stubs) |
| QBM quantum scheduler | `quantum/qbm.c/h`, `quantum/scheduler_advisor.c/h` | SIM |
| User space | `usr/init.c`, `usr/shell.c`, `usr/usr_common.h` | SIM |
| Host bridge | `bridge.c/h` | SIM |
| Brain (small) | `brain.cpp` (+ `brain` binary) | REAL (test binary) |
| Build | `Makefile`, `build.sh`, `build_baremetal_local.sh`, `linker.ld`, `CMakeLists.txt`, `Dockerfile.baremetal` | REAL |
| Tests | `tests/test_*.c` (kernel, scheduler, memory, syscall, multiprocess, interrupts, loader, mach, nt, network, filesystem, rust_driver, linux, bsd, context, fpu_logic, learning) | REAL (hosted) |
