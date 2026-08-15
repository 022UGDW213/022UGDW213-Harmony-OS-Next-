#!/usr/bin/env bash
set -e
# ────────────────────────────────────────────────────────────────────────────
# HarmonyOS Next — REAL bare-metal boot in VNC (fullstack, no mocks)
#
#   kernel : server/native/harmony_kernel — genuine freestanding 32-bit
#            Multiboot kernel (VGA 0xB8000, COM1 UART, PS/2 keyboard drivers)
#   VNC    : 127.0.0.1:5902  (qemu -vnc 127.0.0.1:2 — display quirk: 5900+N)
#   WS     : ws-vnc-bridge 127.0.0.1:6080  path /novnc2  -> 5902
#   View   : http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=localhost&port=6080&path=novnc2
#   serial : /tmp/harmony-serial.log
# ────────────────────────────────────────────────────────────────────────────
ROOT="$(cd "$(dirname "$0")" && pwd)"
NATIVE="$ROOT/server/native"
BRIDGE_DIR="${VNC_BRIDGE_DIR:-/home/jjsp/Desktop/Harmony OS Next}"  # hosts ws-vnc-bridge.js
DISPLAY_N=2
VNC_PORT=$((5900 + DISPLAY_N))

# 1. Build the real kernel (fails hard on any compile error)
echo "🚀 Building HarmonyOS Next bare-metal kernel..."
(cd "$NATIVE" && bash build_baremetal_local.sh) >/dev/null
[ -f "$NATIVE/harmony_kernel" ] || { echo "❌ kernel build failed"; exit 1; }
echo "✅ kernel: $(file -b "$NATIVE/harmony_kernel" | cut -d, -f1-2)"

# 2. Boot in QEMU with VNC (replace any previous instance)
pkill -f "qemu-system-x86_64 .*$NATIVE/harmony_kernel" 2>/dev/null || true
sleep 1
nohup qemu-system-x86_64 -kernel "$NATIVE/harmony_kernel" -m 512M \
    -vnc "127.0.0.1:$DISPLAY_N" -serial file:/tmp/harmony-serial.log \
    -no-reboot > /tmp/harmony-qemu.log 2>&1 &
QEMU_PID=$!
echo "✅ QEMU pid $QEMU_PID — VNC listening on 127.0.0.1:$VNC_PORT"

# 3. Start the WebSocket bridge (if not already running)
if ! pgrep -f "ws-vnc-bridge.js" >/dev/null; then
    (cd "$BRIDGE_DIR" && nohup node ws-vnc-bridge.js > /tmp/vnc-bridge.log 2>&1 &)
    echo "✅ ws-vnc-bridge started (127.0.0.1:6080, /novnc2 -> 5902)"
else
    echo "✅ ws-vnc-bridge already running"
fi

echo ""
echo "▶ VIEW IT — noVNC:"
echo "    http://localhost:3000/novnc/vnc_lite.html?autoconnect=true&host=localhost&port=6080&path=novnc2"
echo "  Serial log: /tmp/harmony-serial.log   (QEMU log: /tmp/harmony-qemu.log)"
