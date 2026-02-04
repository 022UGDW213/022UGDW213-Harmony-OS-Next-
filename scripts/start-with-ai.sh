#!/bin/bash

# Harmony OS Next - AI-Powered Parallel Boot Script
# Starts all intelligent services concurrently

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "🚀 Harmony OS Next - Starting AI-Powered Services..."

# Cleanup function
cleanup() {
    echo ""
    echo "🛑 Shutting down services..."
    kill $(jobs -p) 2>/dev/null || true
    exit 0
}

trap cleanup SIGINT SIGTERM

# Build brain.cpp if not already compiled
if [ ! -f "server/native/brain" ]; then
    echo "🔨 Compiling brain.cpp..."
    cd server/native && make && cd ../..
fi

# Start brain.cpp (Native Intelligence Core)
echo "🧠 Starting Native Intelligence Core (brain.cpp)..."
./server/native/brain "$PROJECT_ROOT" > data/brain-output.log 2>&1 &
BRAIN_PID=$!

# Start Reasoning API (AI Strategic Analysis)
echo "🤖 Starting AI Reasoning API (port 3001)..."
export PATH="/usr/local/bin:$PATH"
node server/reasoning-api.cjs &
REASONING_PID=$!

# Wait for APIs to be ready
sleep 2

# Start Vite Dev Server
echo "⚡ Starting Vite Dev Server (port 54542)..."
npm run dev &
VITE_PID=$!

echo ""
echo "✅ All services started successfully!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🌐 Frontend:      http://localhost:54542/"
echo "🤖 Reasoning API: http://localhost:3001/"
echo "🧠 Brain Core:    Running (PID: $BRAIN_PID)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Press Ctrl+C to stop all services"

# Wait for all background processes
wait
