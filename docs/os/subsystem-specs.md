# Harmony OS Next — Subsystem Specifications

> Detailed specs for the eight core subsystems. Every claim maps to real code
> in `server/native/` (line counts verified 2026-08-15). Status legend:
> **REAL** = boots/executes in the freestanding kernel; **SIM** = hosted
> implementation that compiles and runs on the host (reference for the bare
> port); **PLANNED** = spec-only.

---

## 1. Kernel core

**Files:** `boot/boot.S` (73 ln), `kernel/kmain_bare.c` (72 ln),
`kernel/gdt.c` (50 ln), `kernel/idt.c` (75 ln), `kernel/isr.s` (115 ln),
`drivers/pic.c` (61 ln), `drivers/pit.c` (57 ln), `kernel/kprintf.c` (127 ln),
`kernel/string.c` (52 ln), `linker.ld` (40 ln)
**Status:** REAL

### Interfaces
- `_start` — entry; receives `eax` (MB magic), `ebx` (MB info addr) →
  `kmain(uint32_t magic, uint32_t addr)`.
- `gdt_init()` — loads 3-entry flat GDT (null, code `0x08`/`0x9A`, data
  `0x10`/`0x92`, 4 GiB, 4K gran) via `lgdt` + far jump.
- `idt_init()` — fills 256 gates (`IDT_FLAG_INT_GATE 0x8E`, selector
  `GDT_KERNEL_CODE`), `lidt`.
- `irq_install_handler(int irq, irq_handler_t)` / `irq_get_handler(int irq)`
  — 16-slot IRQ registry.
- `isr_handler(registers_t*)` — dispatch: IRQ 32–47 → hook + `pic_ack(irq)`;
  exception 0–31 → `[CPU] Exception N: name (err=..) at eip=..`; else
  `[CPU] Unhandled interrupt N`.
- `pic_remap(32, 40)` — 8259 ICW1–4; `pic_ack(irq)`; `pic_mask_irq(irq,m)`.
- `pit_init(hz)` (default 100) — channel 0 mode 2, divisor `1193182/hz`;
  `pit_get_ticks()` (u64), `pit_get_seconds()` (u32); `pit_tick` = IRQ0 hook.
- `kprintf(fmt, ...)` — `%s %c %d %u %x %p %%`, zero-pad width, 64-bit hex
  via hi:lo shifts (no 64-bit division under `-nostdlib`).

### Data structures
- `gdt_entry_t[3]`, `gdt_ptr_t`; `idt_entry_t[256]` (base lo/hi, selector,
  flags), `idtr_t`; `registers_t` (int_no, err_code, eip + pusha set);
  `irq_handlers[16]`; `isr_stub_table[48]`.

### Boot sequence (kmain_bare.c)
`serial_init → vga banner → MB magic check → gdt_init → pic_remap(32,40) →
idt_init → pit_init(100) + irq hook 0 + pic_mask_irq(0,0) → meminfo_init →
sti → keyboard_init → shell_run` (never returns; `hlt` fallback).

### Next milestones
1. TSS + ring-3 user mode (GDT gains a user code/data pair).
2. `int 0x80` syscall gate wired to the SIM syscall table.
3. IRQ-driven input (keyboard IRQ1) instead of polling.

**Evidence:** `file server/native/harmony_kernel` (i386 ELF); serial log
`/tmp/harmony-serial.log` shows `[BOOT]`/`[INIT]` sequence; QEMU VNC shows
banner + `harmony>` prompt.

---

## 2. Process manager

**Files:** `kernel/scheduler.c` (250 ln), `kernel/include/process.h`,
`quantum/qbm.c` (72 ln), `quantum/scheduler_advisor.c` (63 ln),
`microkernel/kernel.cpp` (121 ln), `processes/process.cpp` (226 ln),
`asm/context.c` (56 ln), `usr/init.c` (39 ln), `usr/shell.c` (201 ln)
**Status:** SIM (hosted) → PLANNED (bare)

### Interfaces (scheduler.h)
- `scheduler_init()` — clears table, inits QBM.
- `scheduler_add(process_t*)` / `scheduler_remove(int pid)` — 64 slots.
- `scheduler_tick()` — time-slice check (`TIME_SLICE_MS 10`) → quantum
  schedule → context switch.
- `scheduler_yield()` — cooperative yield.
- `quantum_schedule()` — round-robin + QBM boost (`boost_pid`/`boost_factor
  > 0.7`).
- `scheduler_current()`, `scheduler_get_process(pid)`,
  `scheduler_list_processes()`, `scheduler_dump_stats()`.

### Process model
`process_t`: pid, name, state (NEW/READY/RUNNING/BLOCKED/ZOMBIE), uid,
security caps, `last_scheduled`; QBM per-PID row (64×3: CPU-need, I/O-bound,
anomaly; p0/p1 probabilities, `qbm_observe` collapse, learning rate 0.1).

### C++ layer
`harmony::Kernel` — pid table, priority+RR schedule; `harmony::Process`
(worker thread + 100 ms IPC receive loop) with Renderer/GPU/Network
subclasses; `ProcessManager` create/start/stop.

### Hosted user space
`init_main` (PID 1) mounts RamFS (mock), brings up `lo0` (mock), spawns
`shell_main` loop; hsh commands `help ls ps cat whoami su dmesg top qstat
fpu cow mem exit`.

### Bare-metal port plan
- 32-bit `context_switch(prev,next)` asm following `isr.s` style; kernel
  stacks per process (16 KiB default, cf. `boot.S` `stack_bottom`).
- Preemption from PIT IRQ0 (already at 100 Hz): IRQ0 hook →
  `scheduler_tick()`.
- `scheduler_create_process(entry, stack_size)` (signature already used in
  `kernel/main.c` and `mach_task.c`).
- **Evidence:** hosted `test_scheduler`, `test_multiprocess`, `test_fpu_logic`
  (real assertions); bare `uptime` shows the timer driving the clock.

---

## 3. Memory allocator

**Files:** `kernel/meminfo.c` (98 ln), `kernel/memory.c` (254 ln),
`kernel/syscall.c` (mmap/brk sections), `kernel/hal/bare/hal.c` (memory ops)
**Status:** REAL detection, SIM allocator, PLANNED paging

### Detection (REAL)
`meminfo_init(mb_info_addr)` parses MB1:
- `mem_lower`/`mem_upper` (KiB) → `detected_total_kb`;
- mmap entries (packed 24 B + size, type 1 = available) → available MiB +
  entry count; prints each region (`[MEM] base=.. len=.. type=..`).

### Allocator (SIM)
- `memory_init()` — hosted: 1 GiB fake pool; bare: 4 MiB early heap at
  `0x100000` (bump, 16-byte aligned, `memory_early_alloc`).
- `kmalloc(size)`/`kfree(ptr)`/`kmalloc_aligned(size,align)` → HAL
  `HAL_MALLOC/HAL_FREE`; stats (total/used/free, alloc/free counts);
  `memory_get_stats`, `memory_dump_stats`, `memory_validate`, `memory_test`.
- Bare HAL `alloc` currently panics (`[HAL] Memory allocator not
  initialized!`) — allocator is the top porting priority.

### VM syscalls (SIM)
`sys_mmap(addr,len,prot,flags,fd,off)` → prot→`MEM_PROT_*` →
`alloc_virtual(mm, ...)`; `sys_munmap` → `free_virtual`; `sys_brk` stub.

### Spec for the bare allocator (PLANNED)
1. Frame allocator over `meminfo` mmap regions (exclude 0–1 MiB kernel +
   loaded kernel image + early heap).
2. 4 KiB page tables (`boot.S` already aligns to 4K sections); enable paging
   after `meminfo_init`.
3. `alloc_virtual/free_virtual` from the syscall layer map real frames;
   `kmalloc` becomes a kernel-heap slab on top.
4. COW for future `fork` (shell `cow` demo documents the semantics).

**Evidence:** shell `meminfo` (bare, real values); `tests/test_memory.c`,
`tests/test_syscall.c` (hosted).

---

## 4. Virtualization layer

**Files:** `run_qemu.sh` (8 ln), `boot_vnc.sh` (45 ln), `deploy/build_iso.sh`
(62 ln), `Dockerfile.baremetal`, `Makefile` (`iso` target), `boot/miniboot.S`
**Status:** REAL (QEMU/VNC test fabric + ISO pipeline)

### QEMU targets
- Direct boot: `qemu-system-x86_64 -kernel server/native/harmony_kernel -m
  512M -vnc 127.0.0.1:2 -serial file:/tmp/harmony-serial.log -no-reboot`
  (VNC display `:2` → TCP 5902).
- ISO boot: `grub-mkrescue` with GRUB2 `multiboot2 /boot/harmony_kernel`
  menuentry → `qemu-system-i386 -cdrom harmony.iso -serial stdio`.
- Hosted: `make run-hosted` (host binary), `make run-bare` (QEMU via
  `x86_64-elf` toolchain).

### VNC fabric
`-vnc 127.0.0.1:2` → `ws-vnc-bridge.js` (`127.0.0.1:6080`, path `/novnc2`) →
noVNC `http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=
localhost&port=6080&path=novnc2`. Verified: 720×400 VGA framebuffer capture.

### Next
virtio-net (e1000) device, PCI scan, ACPI (multiboot tags), virtio-blk for
the filesystem.

---

## 5. Network stack

**Files:** `rust/rust_net.h`, `rust/mock_rust.c`, `kernel/main.c`
(`rust_net_init` in init), `usr/init.c` (lo0)
**Status:** SIM stub → PLANNED

### Current
`rust_net_init()` brings up a simulated `lo0` (`127.0.0.1`) in the hosted
init process. No real NIC driver, no frames, no sockets.

### Spec (PLANNED)
1. `kernel/net/eth.c` — Ethernet RX/TX over a QEMU e1000 (`-netdev user`),
   IRQ-driven (IRQ11), DMA ring buffers.
2. `kernel/net/arp.c`, `kernel/net/ipv4.c`, `kernel/net/udp.c` — minimal
   stack; loopback first, then `10.0.2.15` QEMU user-net guest.
3. Socket syscalls (`socket/bind/send/recv`) on the syscall table.
4. Mesh integration: kernel speaks the gateway WS protocol
   (`ws://127.0.0.1:18789/gateway`, `IBOT_TOKEN` challenge-response) so the
   OS becomes a first-class iBot node.
**Evidence (next step):** `ping`-style shell command against `10.0.2.2`; host
`nc -u` exchange.

---

## 6. Filesystem model

**Files:** `rust/rust_fs.h`, `rust/mock_rust.c`, `kernel/syscall.c` (fd table,
`sys_open/read/write/close`), `kernel/main.c` (`rust_fs_init`), `usr/shell.c`
(`ls`/`cat`)
**Status:** SIM stub → PLANNED

### Current
- Hosted init mounts a RamFS at `/` and creates `/welcome.msg`
  (`rust_fs_open("/welcome.msg", 1|4)`).
- `sys_open` → `security_can_open_file` check → host `open(2)`; fd table
  `MAX_FDS 256`, fds 0–2 = stdin/stdout/stderr, next fd from 3.

### Spec (PLANNED)
1. Real ramdisk: carve a region from the mmap (or parse the multiboot
   `mods` list as initrd), mount as `/`.
2. VFS: superblock → inode → file; namei resolution; directory entries
   (`sys_getdents` so `ls` is real).
3. Permissions: 9-bit mode + uid/gid checked in `security_can_open_file`
   (currently a 2-rule mock).
**Evidence (next step):** bare shell `cat /welcome.msg` returns the real
string written at boot.

---

## 7. Security sandbox

**Files:** `security/access_control.c` (62 ln), `security/security.h`,
`kernel/syscall.c` (uid/setuid sections), `kernel/include/process.h`
**Status:** SIM → PLANNED (bare)

### Current (SIM)
- `security_check_capability(proc, cap)` — UID 0 bypass or bit set.
- `security_can_open_file(proc, path, mode)` — root/
  `CAP_DAC_OVERRIDE` bypass; else `sensitive.txt` root-only; rest open.
- `security_can_kill(target, caller)` — root/`CAP_KILL` or same UID.
- `sys_open` returns `-13` (EACCES) on denial; `sys_setuid` requires root or
  `CAP_SETUID`, clears caps when dropping from root.

### Spec (PLANNED)
1. Capability table per syscall (`SYS_* → CAP_*`), enforced in the bare
   syscall gate.
2. Page-level sandbox via paging (per-process page tables + supervisor bits).
3. Sandbox profiles mirroring iBot agent tiers (orchestrator/supervisor/
   worker) — the OS sandbox is the kernel half of the mesh trust boundary.
**Evidence:** hosted `whoami`/`su`/`cat sensitive.txt` outputs; denial logs.

---

## 8. Agent orchestration layer

**Files:** `bridge.c` (35 ln), `bridge.h`, `kernel/main.c`
(`bridge_init`), host `server/*.cjs`, iBot gateway/UI APIs
**Status:** REAL mesh + SIM kernel bridge

### Kernel bridge (SIM)
`bridge_update_stats()` writes `{running, processes, active, switches,
timestamp}` to `/tmp/harmony_kernel_stats.json`; `bridge_init()` is called in
the hosted boot (`kernel/main.c` step 6). This is the /proc-analog that the
Node.js API layer (`server/ai-reasoner.cjs`, `server/intelligence-node.cjs`,
`server/reasoning-api.cjs`) and iBot can consume.

### Control plane (REAL, live)
- Gateway `:18789` — 34 cluster agents (4 orch / 5 supv / 25 worker) over 7
  models; WS protocol with `IBOT_TOKEN` challenge-response; `/api/cluster`.
- Python agents `:4003` — 9 FastAPI agents (v8.3.0-AGI).
- UI `:3000` — 43-node swarm graph, 242-skill catalog (15 categories),
  noVNC host for the OS.
- EC2 `34.192.97.238` — 11 source mounts for the orchestrator domains.

### Spec (PLANNED)
1. Bare kernel exposes stats through a syscall (`sys_dmesg` analog) instead
   of a host file.
2. A `bridged` service (agent or kernel thread) publishes kernel telemetry to
   the gateway WS channel (`chat`/metrics), making the OS a monitored node in
   the 43-node graph.
3. Swarm tasks ("port scheduler to bare metal") become OS backlog items via
   the orchestrator, with the verification commands from `cluster-topology.md`
   as acceptance gates.
