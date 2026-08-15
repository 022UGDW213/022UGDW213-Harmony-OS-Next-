# Harmony OS Next — Build Pipeline (real toolchain)

> The exact, verified toolchain and scripts that produce and boot the real
> freestanding kernel. All paths relative to the repo root
> `/home/jjsp/Desktop/Harmony OS Next/Harmony-OS-Next` (note: the path
> contains spaces — every script quotes it).

---

## 1. Toolchain

| Tool | Purpose | Detection order (`build_baremetal_local.sh`) |
|---|---|---|
| `x86_64-elf-gcc` / `x86_64-elf-ld` | cross compiler+linker (Homebrew) | `/usr/local/bin/x86_64-elf-gcc` → `command -v` |
| `gcc` / `ld` (system, with multilib) | fallback via `-m32` | last resort (needs `gcc-multilib` on Ubuntu) |
| `nasm` | hosted Makefile assembler var (not used by the bare script) | `Makefile` `AS` |
| `qemu-system-x86_64` | emulator/VM | `boot_vnc.sh`, `run_qemu.sh`, `make run-bare` |
| `grub-mkrescue` | ISO builder | `deploy/build_iso.sh` |

## 2. Compiler/linker flags (from `build_baremetal_local.sh`)

```bash
CFLAGS="-ffreestanding -O2 -Wall -Wextra -m32 -I. -I./kernel -I./drivers -I./boot -fno-pie"
LDFLAGS="-m elf_i386 -T linker.ld -nostdlib"
# assembler entry (boot.S needs the C preprocessor for multiboot.h):
$CC -x assembler-with-cpp $CFLAGS -c boot/boot.S -o build/boot.o
# ISR stubs are plain gas (no preprocessor):
$CC $CFLAGS -c kernel/isr.s -o build/isr.o
# 14 bare-metal sources compiled individually, then:
$LD -n -o harmony_kernel $LDFLAGS build/*.o
```

The 14 real bare-metal sources: `kernel/kmain_bare.c`, `boot/multiboot.c`,
`drivers/{vga,serial,keyboard,pic,pit}.c`, `kernel/{gdt,idt,kprintf,meminfo,
string,shell}.c`, `kernel/hal/bare/hal.c` — plus `boot/boot.o` and `isr.o`.

`Makefile` alternative (cross + hosted): `make BUILD=bare harmony_bare`
(`-DHARMONY_BARE_METAL`, `-mno-red-zone -fno-stack-protector -nostdlib
-fno-builtin`) or hosted `harmony_hosted` (`-DHARMONY_HOSTED`, links libc).

## 3. Linker script (`linker.ld`, 40 lines)

```ld
ENTRY(_start)
SECTIONS {
    . = 1M;                 /* kernel loads at 1 MiB              */
    .boot : { KEEP(*(.multiboot_header)) }   /* first 8 KiB for QEMU scan */
    .text  : { *(.text) *(.text.*) }
    .rodata: { *(.rodata) *(.rodata.*) }
    .data  : { *(.data) *(.data.*) }
    .bss   : { *(COMMON) *(.bss) *(.bss.*) }
    /DISCARD/ : { *(.note.*) *(.comment) }
}
```

`boot.S` keeps the MB1 header (`0x1BADB002`, flags `0x03`) in
`.multiboot_header` and the 16 KiB stack in `.bss` — QEMU `-kernel` scans the
first 8 KiB of the file for the header, so header-first layout is mandatory.

## 4. Boot scripts

### `./boot_vnc.sh` (one-command fullstack boot)
1. `(cd server/native && bash build_baremetal_local.sh)` — fails hard on
   compile error; checks `harmony_kernel` exists; prints `file` geometry.
2. Kills any previous QEMU instance, then:
   `qemu-system-x86_64 -kernel server/native/harmony_kernel -m 512M
   -vnc 127.0.0.1:2 -serial file:/tmp/harmony-serial.log -no-reboot &`
   → VNC TCP port **5902** (display `:2`).
3. Starts `ws-vnc-bridge.js` (`127.0.0.1:6080`, path `/novnc2`) if not
   running.
4. Prints the noVNC URL:
   `http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=localhost&port=6080&path=novnc2`

### `./run_qemu.sh`
Build (same script) then `qemu-system-x86_64 -kernel server/native/harmony_kernel
-m 512M -vnc 127.0.0.1:2 -serial stdio -no-reboot` — serial on stdio.

### `deploy/build_iso.sh`
`i686-elf-gcc` (else `gcc`), `make harmony_bare.elf`, GRUB2 config with
`multiboot2 /boot/harmony_kernel`, `grub-mkrescue -o harmony.iso isodir/`,
test with `qemu-system-i386 -cdrom harmony.iso -serial stdio`.

### `Makefile` targets
`all` (hosted+bare), `harmony_hosted`, `harmony_bare`, `run-hosted`,
`run-bare` (QEMU), `iso` (grub-mkrescue), `clean`.

## 5. VNC / noVNC chain (real)

```
QEMU -vnc 127.0.0.1:2  (TCP 5902)
   │  RFB
ws-vnc-bridge.js  (127.0.0.1:6080, path /novnc2)
   │  WebSocket
noVNC  http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=localhost&port=6080&path=novnc2
```

Verified 720×400 VGA framebuffer capture over this chain; serial boot log at
`/tmp/harmony-serial.log`, QEMU log at `/tmp/harmony-qemu.log`.

## 6. Verification commands

```bash
file server/native/harmony_kernel      # ELF 32-bit LSB executable, Intel 80386
tail /tmp/harmony-serial.log           # [BOOT]/[INIT] sequence + [MEM] regions
pgrep -af qemu-system-x86_64           # QEMU running
pgrep -af ws-vnc-bridge.js             # bridge running
# hosted reference build + subsystem tests:
make -C server/native harmony_hosted
./server/native/tests/test_scheduler   # etc. (see server/native/tests/)
```

## 7. Known gotchas (from real sessions)

- The repo path has spaces: always quote; `ai-env`-style shebangs break, so
  run scripts via `bash script.sh`, never directly.
- `-nostdlib` forbids 64-bit division — `kprintf.c` prints 64-bit values as
  hi:lo hex and `pit.c` keeps a 32-bit seconds counter (`pit_tick_count`).
- `-m32` needs multilib (`gcc-multilib`); otherwise use `x86_64-elf-gcc`.
- `boot.S` must be compiled with `-x assembler-with-cpp` (multiboot.h
  defines); `isr.s` must NOT (plain gas).
- VNC display `:N` ⇒ port `5900+N` (`:2` ⇒ 5902).
- QEMU `-kernel` direct boot uses the **Multiboot-1** header; the
  **Multiboot-2** path (`boot/miniboot.S` / `multiboot_parse`) needs GRUB/ISO.
