#!/bin/bash
set -e
# Real HarmonyOS Next boot: build the freestanding multiboot kernel, then
# boot it in QEMU with VNC on display :2 (port 5902) + serial to stdio.
cd "$(dirname "$0")"
bash -c 'set -e; bash "$1/build_baremetal_local.sh" >/dev/null' _ server/native
exec qemu-system-x86_64 -kernel server/native/harmony_kernel -m 512M \
    -vnc 127.0.0.1:2 -serial stdio -no-reboot
