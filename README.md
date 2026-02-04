# Harmony OS Next

**AI-Powered Intelligent Desktop Environment**
[![Security Status](https://img.shields.io/badge/Security-Active-green?style=flat&logo=security)](SECURITY.md)
[![AI Reasoning](https://img.shields.io/badge/AI-DeepSeek-blue?style=flat&logo=artificial-intelligence)](docs/AI_REASONING.md)

A beautiful macOS-like desktop simulation enhanced with advanced AI reasoning and security intelligence capabilities from CyberLab.

## 🛡️ Security Architecture
> **"Security is not a feature, it's the kernel."** - *CyberLab Philosophy*

- **Vulnerability Management**: Adheres to strict [Security Policy](SECURITY.md).
- **Automated Hygiene**: `scan.sh` enforces codebase integrity pre-commit.
- **Self-Healing**: `self_heal.sh` automatically recovers from build corruptions.
- **Threat Detection**: Integrated AI agents monitor runtime anomalies (see [AI Reasoning](docs/AI_REASONING.md)).

## ✨ Features

### 🖥️ Desktop Environment
- **macOS-Style Interface** - Beautiful, intuitive desktop with dock and windows
- **Draggable Windows** - Smooth window management with minimize, maximize, close
- **Multiple Wallpapers** - 6 stunning wallpaper options
- **Built-in Apps** - Finder, Weather, Photos, Music, Calendar, Settings, and more

### 🧠 AI Intelligence
- **AI Strategic Reasoner** - Advanced security analysis powered by DeepSeek AI
- **Parallel Consensus** - Multi-model reasoning with DeepSeek + Nova + Kimi
- **Risk Assessment** - Automated threat level evaluation with confidence scoring
- **Threat Predictions** - Predictive analysis of emerging security patterns

### 📊 System Monitoring
- **Native Intelligence Core** - C++ brain.cpp for real-time system metrics
- **Resource Monitoring** - CPU, RAM, and process tracking
- **Intelligence Insights** - Self-reflective system analysis
- **Real-time Updates** - Live data streaming and visualization

### 🎮 Interactive Apps
- **Hybrid Simulator** - Advanced vehicle hybrid system simulation
- **Physics Sandbox** - Interactive ball physics with gravity
- **C++ Engine** - Real-time physics simulation
- **Code Editor** - Polyglot architecture documentation

## 🚀 Quick Start

### Prerequisites
- Node.js (v25.5.0 or later)
- npm (v11.8.0 or later)
- C++ compiler (g++ with C++17 support)
- macOS (for brain.cpp system metrics)

### Installation

```bash
# Clone or navigate to the project
cd 022UGDW213-Harmony-OS-Next--main

# Install dependencies
npm install

# Build native intelligence core
npm run build:brain
```

### Running the App

**Standard Mode** (Frontend only):
```bash
npm run dev
```
Visit http://localhost:54542/

**AI-Powered Mode** (All services):
```bash
npm run dev:ai
```
This starts:
- Frontend (Vite) on port 54542
- Reasoning API on port 3001
- Brain.cpp native core

## 🤖 AI Setup (Optional)
To enable AI-powered features, get an API key from one of these providers:

### DeepSeek (Recommended)
1. Visit [platform.deepseek.com](https://platform.deepseek.com)
2. Create an account and generate an API key
3. Copy `.env.example` to `.env`
4. Add your key: `DEEPSEEK_API_KEY=sk-your-key-here`

### Nova AI (Optional - Parallel Consensus)
1. Get access to Amazon Bedrock Nova
2. Add to `.env`: `NOVA_API_KEY=your-nova-key`

### Kimi AI (Optional - Fallback)
1. Visit [moonshot.cn](https://moonshot.cn)
2. Add to `.env`: `KIMI_API_KEY=your-kimi-key`

## 📱 Available Apps

| App | Description |
|-----|-------------|
| **AI Reasoner** | Strategic security analysis with DeepSeek/Nova AI |
| **System Monitor** | Real-time system metrics from brain.cpp |
| **Finder** | File browser with Lake Tahoe themed items |
| **Weather** | Lake Tahoe weather information |
| **Installer** | Bootable installer creation simulator |
| **Photos** | Photo gallery (placeholder) |
| **Music** | Music player (placeholder) |
| **Amazon Music** | External link to Amazon Music |
| **Calendar** | Calendar app (placeholder) |
| **Settings** | Desktop wallpaper customization |
| **System Profiler** | macOS Tahoe system information |
| **Code Editor** | Polyglot backend architecture docs |
| **Hybrid Sim** | Advanced hybrid vehicle simulation |
| **Sandbox** | Interactive physics sandbox |
| **C++ Engine** | Real-time physics simulation |

## 🛠️ Development

### Project Structure

```
022UGDW213-Harmony-OS-Next--main/
├── server/              # Backend services
│   ├── native/         # C++ brain.cpp
│   ├── ai-reasoner.cjs
│   ├── intelligence-node.cjs
│   └── reasoning-api.cjs
├── scripts/            # Automation scripts
│   └── start-with-ai.sh
├── data/               # Runtime data
├── docs/               # Documentation
│   ├── ARCHITECTURE.md
│   └── AI_REASONING.md
├── src/                # React frontend
│   ├── services/       # API clients
│   └── utils/          # Utilities
├── components/         # React components
│   ├── icons/          # App icons
│   └── apps/           # App content
├── App.tsx             # Main application
└── package.json
```

### Scripts

```bash
# Development
npm run dev              # Start frontend only
npm run dev:ai           # Start all services (AI-powered)
npm run reasoning-api    # Start reasoning API only

# Build
npm run build            # Build frontend for production
npm run build:brain      # Compile brain.cpp
npm run preview          # Preview production build
```

### Adding New Apps

1. Create icon component in `components/icons/`
2. Create content component in `components/apps/`
3. Import both in `App.tsx`
4. Add to `appConfig` object

## 📚 Documentation

- [Architecture](docs/ARCHITECTURE.md) - System design and data flow
- [AI Reasoning](docs/AI_REASONING.md) - AI integration guide

## 🔧 Technology Stack

### 💻 Programming Languages
- **C** (Kernel, Drivers, HAL) - Core operating system logic
- **Assembly (x86)** (Bootloader, Context Switching) - Low-level hardware control
- **TypeScript** (Frontend, Services) - Modern web-based desktop interface
- **C++17** (Native Core) - High-performance system monitoring (brain.cpp)
- **Shell** (DevOps) - Automation and build scripts (`scan.sh`, `self_heal.sh`)
- **Makefile** (Build System) - Cross-platform compilation orchestration

**Frontend:**
- React 19.2 + TypeScript
- Vite 6.2 (dev server & build)
- react-draggable (window management)
- Axios (HTTP client)

**Backend:**
- Node.js + Express
- DeepSeek AI API
- Nova AI (optional)
- Kimi AI (optional)

**Native:**
- C++17 (brain.cpp)
- macOS syscalls

## 🎨 Customization

### Wallpapers

Open Settings app to choose from:
- Tahoe Blue (default)
- Sierra Sunset
- Pine Forest
- Starry Night
- Emerald Bay
- Fresh Powder

### Window Management

- **Drag**: Click and drag window header
- **Minimize**: Yellow button (hides window)
- **Maximize**: Green button (fullscreen)
- **Close**: Red button (closes window)

## 🐛 Troubleshooting

### brain.cpp won't compile

**Issue**: Missing C++ compiler or wrong version

**Solution**:
```bash
# Check compiler
g++ --version

# Install if needed (macOS)
xcode-select --install
```

### AI Reasoner shows "Offline"

**Issue**: Reasoning API not running

**Solution**:
```bash
# Start with AI mode
npm run dev:ai

# Or start API separately
npm run reasoning-api
```

### Port already in use

**Issue**: Port 54542 or 3001 occupied

**Solution**:
```bash
# Find and kill process
lsof -ti:54542 | xargs kill -9
lsof -ti:3001 | xargs kill -9
```

## 🤝 Contributing

This project combines:
- **Harmony OS Next** - Desktop UI simulation
- **CyberLab** - AI reasoning and security intelligence

## 📄 License

Created by Juan Jose Serrano Palacios

## 🔗 Related Projects

- [CyberLab](https://d1547hb5pfnc01.cloudfront.net) - Security dashboard
- Original Harmony OS Next simulation

## 🙏 Acknowledgments

- DeepSeek for advanced AI reasoning
- CyberLab for security intelligence architecture
- React and Vite communities
