# Harmony OS Next

> HarmonyOS NEXT — real fullstack boot (freestanding kernel in QEMU + VNC) and development resources by [022UGDW213 (Time Loops)](https://github.com/022UGDW213)

## ⚡ Real Boot — Bare-Metal Kernel in QEMU + VNC

This repo contains a **genuinely freestanding 32-bit Multiboot kernel** (`server/native/`) with real
hardware drivers — VGA text framebuffer (`0xB8000`), COM1 UART (`0x3f8`), PS/2 keyboard (`0x60/0x64`).
It **really boots** in QEMU and is viewable over VNC. No mocks, no hosted-simulation stand-ins.

```bash
# One command: build kernel → boot QEMU with VNC → start WebSocket bridge
./boot_vnc.sh
```

**View it in a browser (noVNC):**

```
http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=localhost&port=6080&path=novnc2
```

View path: `QEMU -vnc 127.0.0.1:2` (port 5902) → `ws-vnc-bridge` (`127.0.0.1:6080`, path `/novnc2`) → noVNC.

### Boot scripts

| Script | What it does |
|--------|--------------|
| `boot_vnc.sh` | Build the kernel, boot QEMU with VNC `:2`, start the WS bridge, print the noVNC URL |
| `run_qemu.sh` | Build the kernel and boot it in QEMU with VNC `:2` + serial on stdio |

### Kernel layout (`server/native/`)

| Component | File | Details |
|-----------|------|---------|
| Multiboot 1 header + entry | `boot/boot.S` | MB1 magic `0x1BADB002`, stack setup, `call kmain` |
| Kernel main | `kernel/kmain_bare.c` | Driver init, Multiboot magic check, interactive keyboard loop |
| VGA driver | `drivers/vga.c` | Text mode `0xB8000`, 80×25, scrolling |
| Serial driver | `drivers/serial.c` | COM1 `0x3f8`, UART 38400 baud init, putc/print |
| Keyboard driver | `drivers/keyboard.c` | PS/2 ports `0x60/0x64`, scancode set 1 |
| HAL | `kernel/hal/bare/hal.c` | Bare-metal hardware abstraction layer |
| Linker script | `linker.ld` | Loads at 1 MB, multiboot header first |

Build: `bash build_baremetal_local.sh` (or `make BUILD=bare`) from `server/native/` — needs a `-m32`
freestanding toolchain (`gcc-multilib` on Ubuntu; `x86_64-elf-gcc` if present).

Verified: real COM1 serial boot output and a captured 720×400 VGA framebuffer over VNC.

## About

Development resources, code samples, and documentation for **HarmonyOS NEXT** — Huawei's
next-generation distributed operating system.

## Features

- **Real bare-metal boot** (above) — freestanding kernel in QEMU, viewable over VNC
- ArkTS/ArkUI component examples
- Distributed capabilities demos
- Device connectivity patterns
- System service integration

## Web App

The repo also ships a Vite + React web app (ArkTS-style simulator components). Build with
`npm install && npm run build` (Node 22+, note: on exFAT drives invoke `node node_modules/vite/bin/vite.js build`).

## Tech Stack

| Technology | Purpose |
|-----------|----------|
| C (freestanding) | Bare-metal kernel (VGA/serial/keyboard drivers) |
| ArkTS | Primary language |
| ArkUI | UI framework |
| React + Vite | Web app |
| QEMU | x86_64 emulation + VNC display |

## Connect

- GitHub: [@022UGDW213](https://github.com/022UGDW213)
- Website: [o22ugdw213.network](https://sites.google.com/view/o22ugdw213/home)
- YouTube: [@O22UGDW213](https://youtube.com/@O22UGDW213)

## License

MIT License — see [LICENSE](LICENSE)
