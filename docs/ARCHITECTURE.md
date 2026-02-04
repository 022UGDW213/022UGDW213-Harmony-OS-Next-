# Harmony OS Next - System Architecture

## Overview

Harmony OS Next is an intelligent, AI-powered desktop environment that combines a beautiful macOS-like UI with advanced security intelligence capabilities from CyberLab.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Frontend (React + Vite)                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ AI Reasoner  │  │System Monitor│  │  Other Apps  │      │
│  │     App      │  │     App      │  │  (Finder,    │      │
│  │              │  │              │  │   Weather,   │      │
│  └──────┬───────┘  └──────┬───────┘  │   etc.)      │      │
│         │                 │          └──────────────┘      │
│         │                 │                                 │
│         └─────────┬───────┘                                 │
│                   │                                         │
│            ┌──────▼──────┐                                  │
│            │ AI Service  │                                  │
│            │  (Axios)    │                                  │
│            └──────┬──────┘                                  │
└───────────────────┼──────────────────────────────────────────┘
                    │ HTTP/REST
┌───────────────────▼──────────────────────────────────────────┐
│                  Backend Services (Node.js)                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │           Reasoning API (Express)                     │   │
│  │  Port: 3001                                           │   │
│  │  Endpoints:                                           │   │
│  │    - POST /api/reasoning/analyze                      │   │
│  │    - POST /api/reasoning/threat                       │   │
│  │    - GET  /health                                     │   │
│  └────────────────┬─────────────────────────────────────┘   │
│                   │                                          │
│  ┌────────────────▼─────────────────────────────────────┐   │
│  │         AI Reasoner (ai-reasoner.cjs)                │   │
│  │  - DeepSeek integration (reasoning model)            │   │
│  │  - Nova AI parallel consensus                        │   │
│  │  - Kimi AI fallback                                  │   │
│  │  - Multi-model result merging                        │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │    Intelligence Node (intelligence-node.cjs)         │   │
│  │  - CVE pattern extraction                            │   │
│  │  - Threat categorization                             │   │
│  │  - Risk assessment                                   │   │
│  │  - Pattern clustering                                │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│         Native Intelligence Core (C++)                        │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              brain.cpp                                │   │
│  │  - System metrics monitoring (CPU, RAM, processes)   │   │
│  │  - Self-reflection on scan history                   │   │
│  │  - Line-delimited JSON stream output                 │   │
│  │  - Real-time intelligence insights                   │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│                    External AI Services                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  DeepSeek    │  │  Nova AI     │  │   Kimi AI    │      │
│  │  (Primary)   │  │  (Parallel)  │  │  (Fallback)  │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────────────────────────────────────────────┘
```

## Data Flow

### 1. AI Strategic Analysis Flow

```
User clicks "Request Analysis" in AI Reasoner App
    ↓
Frontend calls aiService.requestStrategicAnalysis()
    ↓
HTTP POST to http://localhost:3001/api/reasoning/analyze
    ↓
Reasoning API receives request
    ↓
ai-reasoner.cjs processes request:
    - Constructs prompts with security telemetry
    - Calls DeepSeek API (primary)
    - Calls Nova API (parallel, if configured)
    - Calls Kimi API (fallback, if needed)
    ↓
Multiple AI responses received
    ↓
Results merged into consensus (if multiple providers)
    ↓
Strategic analysis returned to frontend
    ↓
AI Reasoner App displays:
    - Strategic briefing
    - Risk assessment
    - Recommendations
    - Threat predictions
    - Confidence score
```

### 2. System Monitoring Flow

```
System Monitor App mounts
    ↓
Component starts interval timer (2s)
    ↓
Reads brain.cpp output stream (simulated in current version)
    ↓
Parses JSON metrics:
    - RAM usage
    - Active tasks
    - Intelligence insights
    ↓
Updates UI with real-time data
    ↓
Displays resource graphs and insights
```

## Component Responsibilities

### Frontend Components

**AI Reasoner App (`components/apps/AIReasonerContent.tsx`)**
- User interface for AI strategic analysis
- API health monitoring
- Request/response handling
- Data visualization (risk levels, confidence scores)

**System Monitor App (`components/apps/SystemMonitorContent.tsx`)**
- Real-time system metrics display
- Brain.cpp data stream integration
- Resource usage visualization
- Intelligence insights presentation

**AI Service (`src/services/aiService.ts`)**
- HTTP client for reasoning API
- Type-safe interfaces
- Error handling and fallbacks
- Health check functionality

### Backend Services

**Reasoning API (`server/reasoning-api.cjs`)**
- Express.js REST API server
- CORS configuration for frontend access
- Request validation
- Response formatting
- Error handling

**AI Reasoner (`server/ai-reasoner.cjs`)**
- Multi-provider AI integration
- Prompt engineering for security analysis
- Parallel consensus building
- Result merging and deduplication
- Confidence scoring

**Intelligence Node (`server/intelligence-node.cjs`)**
- CVE database pattern extraction
- Threat keyword matching
- Category-based threat analysis
- Risk assessment algorithms
- Pattern clustering

### Native Core

**brain.cpp (`server/native/brain.cpp`)**
- System metrics collection via syscalls
- Scan history analysis
- Pattern-based insights
- JSON stream output
- Continuous monitoring loop

## Technology Stack

### Frontend
- **React 19.2** - UI framework
- **TypeScript** - Type safety
- **Vite 6.2** - Build tool and dev server
- **Axios** - HTTP client
- **Tailwind CSS** - Styling (via inline classes)

### Backend
- **Node.js** - Runtime environment
- **Express 4.18** - Web framework
- **Axios** - HTTP client for AI APIs
- **dotenv** - Environment configuration

### Native
- **C++17** - Native intelligence core
- **macOS syscalls** - System metrics

### AI Services
- **DeepSeek** - Primary reasoning model
- **Nova AI** - Parallel consensus (optional)
- **Kimi AI** - Fallback provider (optional)

## Configuration

### Environment Variables

Create a `.env` file in the project root:

```bash
# AI API Keys (at least one required for AI features)
DEEPSEEK_API_KEY=sk-your-deepseek-key
NOVA_API_KEY=your-nova-key  # Optional
KIMI_API_KEY=your-kimi-key  # Optional

# Server Ports
REASONING_API_PORT=3001
VITE_DEV_PORT=54542
```

## Deployment

### Development Mode

```bash
# Standard mode (frontend only)
npm run dev

# AI-powered mode (all services)
npm run dev:ai
```

### Production Build

```bash
# Build frontend
npm run build

# Build native brain
npm run build:brain

# Preview production build
npm run preview
```

## Security Considerations

1. **API Keys**: Store securely in `.env`, never commit to version control
2. **CORS**: Configured for localhost development, adjust for production
3. **Rate Limiting**: Consider implementing for AI API calls
4. **Input Validation**: All user inputs sanitized before AI processing
5. **Error Handling**: Graceful degradation when services unavailable

## Performance

- **Frontend**: Lazy loading, code splitting via Vite
- **Backend**: Async/await for non-blocking operations
- **Native**: Optimized C++ with minimal overhead
- **Caching**: Consider implementing for AI responses

## Monitoring & Debugging

- **Frontend**: Browser DevTools, React DevTools
- **Backend**: Console logs, error tracking
- **Native**: Output logs in `data/brain-output.log`
- **API**: Health check endpoint at `/health`
