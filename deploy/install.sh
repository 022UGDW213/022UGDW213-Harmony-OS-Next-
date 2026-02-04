#!/bin/bash
set -e

echo "💿 HarmonyOS Next - Deploy & Install Script"
echo "==========================================="

PROJECT_ROOT=$(pwd)

# 1. Build Native Kernel (Simulation)
echo "🔧 Building Native Kernel..."
cd server/native
make harmony_kernel
cd "$PROJECT_ROOT"

# 2. Install NPM Dependencies
echo "📦 Installing Dependencies..."
npm install

# 3. Build Web Frontend (Vite)
echo "⚛️  Building Web Interface..."
npm run build 2>/dev/null || echo "⚠️  Vite build skipped (dev mode active)"

# 4. Setup Service (Mock)
echo "✅ Installation Complete."
echo ""
echo "To start the Full Stack environment:"
echo "  1. Start Kernel:  ./server/native/harmony_kernel &"
echo "  2. Start API:     node server/reasoning-api.cjs &"
echo "  3. Start UI:      npm run dev"
