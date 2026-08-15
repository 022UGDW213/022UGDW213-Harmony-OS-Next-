#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────
# build-hbin.sh — build the bare-metal kernel and pack it into
# the .hbin unified binary format (docs/os/bin-format.md).
#
# Output:  <repo>/build/harmony-kernel.hbin
# Tools:   tools/hbin_pack.py (packer/reader)
#
# Usage:   bash scripts/build-hbin.sh
#          KERNEL_SRC=<elf> HBIN_OUT=<file.hbin> bash scripts/build-hbin.sh
# ─────────────────────────────────────────────────────────────
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KERNEL_SRC="${KERNEL_SRC:-server/native/harmony_kernel}"
HBIN_OUT="${HBIN_OUT:-$ROOT/build/harmony-kernel.hbin}"
PACK="$(command -v python3 || true)"

if [ -z "$PACK" ]; then
    echo "❌ python3 not found" >&2
    exit 1
fi

echo "🚀 [1/2] Building bare-metal kernel..."
(cd "$ROOT/server/native" && bash build_baremetal_local.sh)

if [ ! -f "$ROOT/$KERNEL_SRC" ]; then
    echo "❌ kernel image not found: $ROOT/$KERNEL_SRC" >&2
    exit 1
fi

echo "🚀 [2/2] Packing $ROOT/$KERNEL_SRC -> $HBIN_OUT"
mkdir -p "$(dirname "$HBIN_OUT")"
python3 "$ROOT/tools/hbin_pack.py" "$ROOT/$KERNEL_SRC" -o "$HBIN_OUT" --info

echo "✅ Packed: $HBIN_OUT"
