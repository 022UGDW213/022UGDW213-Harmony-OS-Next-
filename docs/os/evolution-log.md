# Harmony OS Next — Evolution Log

> Chronological record of the OS development, from the git history of this
> repo (verified 2026-08-15, branch `main`) and the current session. Each
> phase names the real commits, files, and evidence. Phases marked
> **SIM** produced hosted implementations that compile and run on the host;
> phases marked **REAL** produced code that boots in QEMU.

---

## Phase 0 — Repo genesis (2025-11-21)
**Commits:** `7954e8c`, `12d175f`, `6b2e4a4` ("Add files via upload",
"Add homepage link to README")
Early uploads of the HarmonyOS Next resource bundle (ArkTS samples, docs,
design files). No kernel.

## Phase 1 — Microkernel proto (2026-01-31 … 2026-02-04)
**Commits:** `53d7719` ("updated app"), `6365a85` ("feat: Phase 13 Kernel
Convergence (Unified Log/Mem/Sched) + Branding + Tools"), `f33b03e` (merge),
`39b8da9`, `69155fb` (docs: languages, security policy, architecture)
**Files created (SIM):** `kernel/hal/*` (HAL abstraction), `kernel/log.c`
(circular 4 KiB dmesg buffer), `kernel/memory.c` (HAL allocator + stats),
`kernel/scheduler.c` (quantum scheduler + QBM), `quantum/qbm.c`,
`quantum/scheduler_advisor.c`, `microkernel/{kernel,ipc}.cpp`, `mach/*`,
`nt/*`, `processes/process.cpp`, `security/access_control.c`, `loader/`,
`linux/namespace.c`, `rust/*` stubs, `usr/{init,shell}.c`, `bridge.c`,
`tests/*`.
**Evidence:** hosted test binaries (`test_kernel`, `test_scheduler`,
`test_memory`, `test_mach`, `test_nt`, …) with real assertions; hosted boot
sequence in `kernel/main.c` (IDT→syscall→memory→scheduler→init→bridge→
schedule).

## Phase 2 — Real fullstack boot, Multiboot-1 (2026-08-15)
**Commits:** `b76393c` ("Real fullstack boot: fix hal.c includes +
Multiboot-1 boot, VNC boot scripts"), `d0189d8` ("document real fullstack
boot, rename package, deps audited"), `a9aa963` (README by iBot SI)
**What landed (REAL):**
- `boot/boot.S` — MB1 header (`0x1BADB002`) + `_start` → `kmain`.
- `kernel/kmain_bare.c` — real boot sequence.
- `boot_vnc.sh` / `run_qemu.sh` — one-command QEMU + VNC boot.
- HAL bare backend (`kernel/hal/bare/hal.c`) compiles in the freestanding
  build.
**Evidence:** first serial boot log; 720×400 VGA capture over
VNC→ws-vnc-bridge→noVNC.

## Phase 3 — GDT / IDT / PIC / PIT (2026-08-15, HEAD `8c7fa1a`)
**Commit:** `8c7fa1a` ("OS dev: real GDT/IDT/PIC/PIT, memory detection, VGA
scroll, interactive shell")
**What landed (REAL):**
- `kernel/gdt.c` — flat 32-bit GDT (code `0x08`, data `0x10`), real `lgdt`
  + far jump + segment reload.
- `kernel/isr.s` — 48 ISR stubs (error-code vectors 8, 10–14, 16, 17),
  `isr_common_stub`, `isr_stub_table`.
- `kernel/idt.c` — 256 int gates, IRQ registry, exception dispatch.
- `drivers/pic.c` — 8259 remap to vectors 32–47, EOI, per-IRQ masks.
- `drivers/pit.c` — 100 Hz channel-0 rate generator, tick + seconds counters.
- `kernel/kprintf.c` — freestanding printf (serial + VGA, 64-bit safe).
- `kernel/string.c` — memcpy/memmove/memset/strcmp/strlen.
- `kernel/shell.c` — interactive shell (`help/uptime/meminfo/echo/clear/
  reboot`).
**Evidence:** serial `[INIT]` sequence; `sti` then IRQ0 ticks flow; shell
`uptime` counts real PIT seconds.

## Phase 4 — Memory detection (meminfo) (2026-08-15, same commit)
**What landed (REAL):** `kernel/meminfo.c` — MB1 `mem_lower`/`mem_upper` +
BIOS mmap parse; totals and per-region printout; `meminfo` shell command.
**Evidence:** serial `[MEM]` lines with real QEMU RAM values (512 MiB guest).

## Phase 5 — Interactive shell (2026-08-15, same commit)
**What landed (REAL):** full line editor over COM1 RX + PS/2 polling;
commands `help/uptime/meminfo/echo/clear/reboot`; `reboot` pulses the 8042
reset line (`outb 0xFE` → `0x64`).
**Evidence:** `harmony>` prompt live on noVNC and serial.

## Phase 6 — Scheduler / allocator / IPC / device layer (target, in progress)
**Current state:**
- **Scheduler (SIM):** `kernel/scheduler.c` + QBM; hosted context switch only.
- **Allocator (SIM):** `kernel/memory.c` HAL allocator; bare HAL `alloc`
  panics (`[HAL] Memory allocator not initialized!`) — port is the top
  priority.
- **IPC (SIM):** `microkernel/ipc.cpp` queues; Mach/NT shims.
- **Devices (REAL):** VGA/COM1/PS-2/PIC/PIT real; keyboard/alloc HAL TODOs
  open.
**Next milestones (bare-metal):**
1. Physical frame allocator from `meminfo` mmap + `kmalloc` on a real kernel
   heap (unblock bare HAL `alloc`).
2. 32-bit `context_switch` (pattern: `isr.s`) + PIT-IRQ0 preemption →
   `scheduler_create_process` on bare.
3. `int 0x80` syscall gate wired to the SIM syscall table; copy-in/copy-out.
4. Kernel IPC queue; IRQ-driven keyboard (IRQ1).
5. e1000 NIC + minimal IP/UDP; RamFS from an initrd region.
6. Bridge kernel telemetry into the iBot gateway (34-agent mesh) so the OS
   is a monitored node in the 43-node graph.

## Phase 7 — This documentation session (2026-08-15)
Authored `docs/os/`: `architecture.md`, `cluster-topology.md`, `agent-roles.md`,
`subsystem-specs.md`, `build-pipeline.md`, `evolution-log.md` (this file).
Verified live cluster numbers (34 gateway + 9 python agents = 43 nodes; 242
catalog skills; 685 on-disk packs) against the auth-gated APIs. No code
changed.

---

## Current git state
```
* 8c7fa1a 2026-08-15 OS dev: real GDT/IDT/PIC/PIT, memory detection, VGA scroll, interactive shell
* d0189d8 2026-08-15 Update + upgrade: document real fullstack boot, rename package, deps audited
* b76393c 2026-08-15 Real fullstack boot: fix hal.c includes + Multiboot-1 boot, VNC boot scripts
* a9aa963 2026-03-21 Update: Enhanced README documentation — deployed by iBot SI
* 69155fb 2026-02-04 docs: add security policy and architecture details
* 39b8da9 2026-02-04 docs: add programming languages list to README
* f33b03e 2026-02-04 merge: resolve conflicts by prioritizing local Phase 13 kernel work
* 6365a85 2026-02-04 feat: Phase 13 Kernel Convergence + Branding + Tools
* 53d7719 2026-01-31 updated app
* 7954e8c / 6b2e4a4 / 12d175f 2025-11-21 initial uploads
```
