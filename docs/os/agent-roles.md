# Harmony OS Next — Agent Roles: Specialized Design Roles & Owners

> Role definition for the OS development swarm. Each role names the real
> source files it owns (all under `server/native/`), the real swarm agents
> that fill it (from the verified 43-node topology: 34 gateway + 9 python
> agents), and the acceptance evidence (real build/run output). Roles follow
> the hybrid architecture: a **REAL freestanding core** and the **SIM/planned**
> layers that get ported onto it.

---

## R1 — Kernel core engineer
**Files:** `boot/boot.S`, `boot/multiboot.c`, `kernel/kmain_bare.c`,
`kernel/gdt.c`, `kernel/idt.c`, `kernel/isr.s`, `kernel/kprintf.c`,
`kernel/string.c`, `linker.ld`
**Swarm agents:** `code-builder` (deepseek-r1:7b), `code-reviewer`,
`code-auditor`, backed by `orchestrator` (gemma3:12b)
**Current state:** REAL — multiboot-1 boot, flat GDT, 256-gate IDT, 48 ISR
stubs, exception dispatch, `kmain_bare` init sequence.
**Next:** TSS + ring-3, `syscall` gate (int 0x80), ELF loader integration
(`loader/loader.c`).
**Evidence:** serial boot log (`/tmp/harmony-serial.log`), `file
harmony_kernel` geometry.

## R2 — Scheduler engineer
**Files:** `kernel/scheduler.c`, `kernel/include/process.h`,
`quantum/qbm.c`, `quantum/scheduler_advisor.c`, `asm/context.c`,
`microkernel/kernel.cpp`
**Swarm agents:** `ml-engineer`, `executor`, `monitor-ops`
**Current state:** SIM — 64-slot quantum scheduler, 10 ms slices, RR + QBM
boost (64×3 qmatrix), hosted context switch.
**Next (bare port):** 32-bit `context_switch` asm (pattern: `isr.s`),
per-process kernel stacks, PIT-IRQ0-driven preemption (100 Hz already live).
**Evidence:** `tests/test_scheduler.c`, `tests/test_multiprocess.cpp`,
`tests/test_fpu_logic.c` (hosted); serial `[CPU]`/uptime output (bare).

## R3 — Memory engineer
**Files:** `kernel/meminfo.c`, `kernel/memory.c`, `kernel/syscall.c` (mmap
section)
**Swarm agents:** `data-analyst`, `etl-worker`, `sql-agent`, `file-manager`
**Current state:** REAL detection (`meminfo.c` mmap parse); SIM allocator
(HAL `kmalloc/kfree`, 4 MiB bump heap at `0x100000`, hosted 1 GiB pool).
**Next:** physical frame allocator from the mmap entries, 4 KiB paging,
`alloc_virtual/free_virtual` on real page tables, kernel heap front-end.
**Evidence:** shell `meminfo` output (bare); `tests/test_memory.c`,
`tests/test_syscall.c` (hosted).

## R4 — IPC engineer
**Files:** `microkernel/ipc.cpp`, `mach/mach_ipc.c`, `mach/mach_task.c`,
`nt/ob_manager.c`, `nt/handle_table.c`, `nt/registry.c`, `nt/executive.c`
**Swarm agents:** `backend-developer`, `network-ops`
**Current state:** SIM — C++ priority message queues; Mach port/task shim;
NT object/handle/registry shim.
**Next (bare):** kernel message queue + `ipc_send/ipc_recv` syscalls with
copy-in/copy-out; keep Mach/NT as compat shims.
**Evidence:** `tests/test_mach.c`, `tests/test_mach_task.c`, `tests/test_nt.c`,
`tests/test_nt_exec.c`.

## R5 — Security engineer
**Files:** `security/access_control.c`, `security/security.h`,
`kernel/syscall.c` (uid/caps), `SECURITY.md`
**Swarm agents:** `vulnerability-scanner`, `pen-tester`, `incident-responder`,
`compliance-agent`
**Current state:** SIM — capability checks (`CAP_DAC_OVERRIDE`, `CAP_KILL`),
UID model in `process_t.security`, `sys_open` EACCES, `setuid` cap-drop.
**Next:** per-syscall capability table, page-level sandbox (with paging),
agent trust boundary.
**Evidence:** `tests/test_syscall.c` (uid/security), shell `whoami`/`su`
(hosted).

## R6 — Virtualization engineer
**Files:** `boot/miniboot.S`, `deploy/build_iso.sh`, `Dockerfile.baremetal`,
`run_qemu.sh`, `boot_vnc.sh`
**Swarm agents:** `devops-engineer`, `cloud-ops`, `container-ops`
**Current state:** REAL — QEMU `-kernel` multiboot-1 direct boot is the test
target; ISO path (multiboot2/GRUB) scripted; VNC display `:2` → 5902 →
`ws-vnc-bridge` (6080, `/novnc2`) → noVNC on `:3000/novnc/...`.
**Next:** virtio-net e1000, virtio-blk, PCI enumeration, ACPI.
**Evidence:** `boot_vnc.sh` noVNC URL works; captured VGA framebuffer
(720×400) and serial log.

## R7 — Device-abstraction engineer
**Files:** `kernel/hal/hal.h`, `kernel/hal/bare/hal.c`,
`kernel/hal/hosted/hal.c`, `kernel/hal/common/*`, `drivers/{vga,serial,
keyboard,pic,pit}.c`, `rust/rust_driver.h`
**Swarm agents:** `frontend-developer`, `design-agent` (HAL UI/console)
**Current state:** REAL — bare HAL backend wires VGA/COM1/PS-2/PIC/PIT with
real `outb/inb`; hosted backend wires libc. `hal_t` struct of function
pointers (console/memory/time/interrupt).
**Next:** fill bare `alloc/getc/register_handler/acknowledge` TODOs;
IRQ-driven keyboard (IRQ1); driver registry.
**Evidence:** hosted `tests/test_hal.c`; bare boot messages through
`serial_print`.

## R8 — Compiler-pipeline engineer
**Files:** `Makefile`, `build.sh`, `build_baremetal_local.sh`, `linker.ld`,
`CMakeLists.txt`, `kernel/hal/common/hal_stdio.c`
**Swarm agents:** `cicd-agent`, `deploy-agent`
**Current state:** REAL — `gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie`,
`-nostdlib` link with `linker.ld` (`-m elf_i386`), `boot.S` via
`-x assembler-with-cpp`, `isr.s` plain gas; two targets (bare `harmony_kernel`
/ hosted `harmony_hosted`); ISO via `grub-mkrescue`.
**Next:** CI matrix (gcc-multilib, x86_64-elf-gcc, i686-elf-gcc), kprintf
format-coverage tests (no 64-bit division in `-nostdlib`).
**Evidence:** exit-0 builds; `file harmony_kernel` shows i386 ELF; QEMU boot
log.

## R9 — Build & release engineer
**Files:** `boot_vnc.sh`, `run_qemu.sh`, `deploy/install.sh`,
`scripts/start-with-ai.sh`, `self_heal.sh`, `scan.sh`
**Swarm agents:** `orchestrator`, `deploy-agent`, `stackops-agent-001`
**Current state:** REAL — one-command boot (`./boot_vnc.sh`), serial log to
`/tmp/harmony-serial.log`, QEMU pidfile management, self-heal/scan helpers.
**Next:** artifact signing (hbin), versioned kernel releases, EC2 mirror.
**Evidence:** noVNC reachable; `pgrep qemu-system-x86_64`; serial log tail.

---

## 4. Role ↔ swarm mapping table (verified agents)

| Role | Lead agents (model) | Secondary agents | Python-side counterpart |
|---|---|---|---|
| R1 kernel core | code-builder (deepseek-r1:7b) | code-reviewer, code-auditor | backend-agent-001 |
| R2 scheduler | ml-engineer (deepseek-r1:7b) | executor, monitor-ops | stackops-agent-001 |
| R3 memory | data-analyst (qwen2.5:7b) | sql-agent, file-manager | backend-agent-001 |
| R4 IPC | backend-developer (deepseek-r1:7b) | network-ops | backend-agent-001 |
| R5 security | vulnerability-scanner (deepseek-r1:7b) | pen-tester, incident-responder | cyberlab-agent-001, aws-iam-agent-001 |
| R6 virtualization | devops-engineer (mistral:7b) | cloud-ops, container-ops | deploy-agent-001 |
| R7 device abstraction | design-agent (llama3.1:8b) | frontend-developer | frontend-agent-001, design-agent-001 |
| R8 compiler pipeline | cicd-agent (mistral:7b) | deploy-agent | qa-agent-001, stackops-agent-001 |
| R9 build & release | orchestrator (gemma3:12b) | stackops-agent-001 | deploy-agent-001 |

## 5. Working rules
1. No mocks: every change must compile in `build_baremetal_local.sh` (bare)
   and/or `make harmony_hosted` (sim), and the effect must be visible in real
   serial/VGA output or a real hosted test binary — never fabricated.
2. A change is "done" only with captured evidence (serial log line, test
   pass, `file` geometry, screenshot).
3. Bare-metal ports of SIM subsystems keep the HAL seam; the hosted build
   stays green as the reference implementation.
4. Docs live in `docs/os/` and must reflect the real code (this set: see
   `architecture.md`, `cluster-topology.md`, `subsystem-specs.md`,
   `build-pipeline.md`, `evolution-log.md`).
