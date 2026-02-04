# AI Reasoning Integration Guide

## Overview

Harmony OS Next integrates advanced AI reasoning capabilities through multiple Large Language Model (LLM) providers to deliver strategic security analysis and threat intelligence.

## Supported AI Providers

### 1. DeepSeek (Primary)

**Model**: `deepseek-reasoner` (with `deepseek-chat` fallback)

**Capabilities**:
- Advanced reasoning and chain-of-thought analysis
- Strategic security assessment
- Risk prediction
- Threat pattern recognition

**Setup**:
1. Get API key from [platform.deepseek.com](https://platform.deepseek.com)
2. Add to `.env`:
   ```bash
   DEEPSEEK_API_KEY=sk-your-api-key-here
   ```

**Pricing**: Highly competitive, excellent value for reasoning tasks

### 2. Nova AI (Parallel Consensus)

**Model**: `nova-pro`

**Capabilities**:
- Parallel analysis alongside DeepSeek
- Consensus building
- Alternative perspective generation
- Supply chain risk analysis

**Setup**:
1. Get API key from Amazon Bedrock
2. Add to `.env`:
   ```bash
   NOVA_API_KEY=your-nova-key-here
   ```

**Note**: If no key provided, system uses mock Nova for demonstration

### 3. Kimi AI (Fallback)

**Model**: `moonshot-v1-8k`

**Capabilities**:
- Fallback when DeepSeek unavailable
- Long-context analysis
- Alternative reasoning approach

**Setup**:
1. Get API key from [moonshot.cn](https://moonshot.cn)
2. Add to `.env`:
   ```bash
   KIMI_API_KEY=your-kimi-key-here
   ```

## How It Works

### Single Provider Mode

When only one API key is configured:

```
User Request → Reasoning API → Single AI Provider → Response
```

The system uses the configured provider and returns its analysis directly.

### Parallel Consensus Mode

When multiple API keys are configured:

```
User Request → Reasoning API → ┌─ DeepSeek ─┐
                                ├─ Nova AI ─┤ → Merge → Consensus Response
                                └─ Kimi AI ─┘
```

The system:
1. Sends requests to all configured providers simultaneously
2. Waits for all responses
3. Merges results into a consensus analysis
4. Combines recommendations and insights
5. Averages confidence scores

### Fallback Mechanism

```
DeepSeek Reasoning Model
    ↓ (if fails)
DeepSeek Chat Model
    ↓ (if fails)
Kimi AI (if configured)
    ↓ (if all fail)
Offline Mode (helpful error message)
```

## API Integration

### Request Format

```typescript
POST /api/reasoning/analyze
Content-Type: application/json

{
  "scanResult": {
    "results": [...],
    "timestamp": "2024-02-03T20:00:00Z"
  },
  "brainInsight": {
    "intelligence": ["System operational"],
    "metrics": {...}
  },
  "learnedPatterns": [...],
  "cveData": [...]
}
```

### Response Format

```typescript
{
  "success": true,
  "status": "online",
  "provider": "deepseek + nova",  // Single or combined
  "modelType": "parallel-consensus",
  "strategicBriefing": "High-level security assessment...",
  "riskAssessment": "MEDIUM",
  "riskReasoning": "Detailed reasoning...",
  "recommendations": [
    "Implement security measure A",
    "Review configuration B",
    "Monitor system C"
  ],
  "threatPredictions": [
    "Potential supply chain attack",
    "Emerging vulnerability pattern"
  ],
  "confidence": 85,
  "keyInsights": [
    "Anomaly detected in pattern X",
    "Trend shift in category Y"
  ],
  "timestamp": "2024-02-03T20:00:05Z"
}
```

## Prompt Engineering

### System Prompt

The AI is instructed to act as a security expert with deep knowledge in:
- Threat landscape analysis
- Vulnerability assessment
- Risk prioritization
- Strategic security planning
- Predictive threat modeling

### User Prompt Structure

```
### SECURITY TELEMETRY ###
[Scan results, brain insights, learned patterns, CVE data]

### REASONING REQUEST ###
Perform deep strategic analysis and provide:
1. Strategic Briefing (2-3 sentences)
2. Risk Assessment (CRITICAL/HIGH/MEDIUM/LOW)
3. Priority Recommendations (3-4 items)
4. Threat Predictions (2-3 items)
5. Confidence Score (0-100)
```

## Response Processing

### JSON Parsing

The system handles multiple response formats:
1. **Pure JSON**: Direct parsing
2. **Markdown-wrapped JSON**: Extracts JSON from code blocks
3. **Plain text**: Fallback formatting

### Result Merging

When combining multiple AI responses:

```typescript
{
  strategicBriefing: "[DEEPSEEK]: Analysis A\n\n[NOVA]: Analysis B",
  riskAssessment: highestRiskLevel,  // CRITICAL > HIGH > MEDIUM > LOW
  recommendations: uniqueRecommendations,  // Deduplicated
  threatPredictions: uniquePredictions,
  confidence: averageConfidence,
  keyInsights: uniqueInsights
}
```

## Usage Examples

### Basic Analysis Request

```typescript
import { requestStrategicAnalysis } from './services/aiService';

const analysis = await requestStrategicAnalysis({
  scanResult: { status: 'healthy' },
  brainInsight: { intelligence: ['System operational'] }
});

console.log(analysis.strategicBriefing);
console.log(analysis.riskAssessment);
```

### Specific Threat Analysis

```typescript
import { analyzeSpecificThreat } from './services/aiService';

const result = await analyzeSpecificThreat({
  cveId: 'CVE-2024-1234',
  description: 'Remote code execution vulnerability...',
  cvssScore: 9.8
});

console.log(result.analysis);
```

### Health Check

```typescript
import { checkReasoningAPIHealth } from './services/aiService';

const isOnline = await checkReasoningAPIHealth();
if (isOnline) {
  console.log('Reasoning API is ready');
}
```

## Configuration Options

### Temperature

Controls randomness in AI responses:
- **0.2**: More deterministic (threat analysis)
- **0.3**: Balanced (strategic analysis)
- **0.7**: More creative (brainstorming)

Current setting: `0.3` for strategic analysis

### Max Tokens

Controls response length:
- **Strategic Analysis**: 2000 tokens
- **Threat Analysis**: 1500 tokens

### Timeout

API request timeout: `15000ms` (15 seconds)

## Error Handling

### API Key Missing

```json
{
  "success": false,
  "status": "offline",
  "reason": "API Key Missing",
  "strategicBriefing": "Strategic reasoning is currently restricted...",
  "recommendations": [
    "Configure DEEPSEEK_API_KEY in .env",
    "Once configured, the system will provide autonomous analysis"
  ]
}
```

### Network Error

```json
{
  "success": false,
  "status": "error",
  "reason": "Network timeout",
  "strategicBriefing": "The AI Reasoner encountered an error...",
  "recommendations": [
    "Verify API key validity",
    "Check network connectivity",
    "Review system logs"
  ]
}
```

### Rate Limiting

The system gracefully handles rate limits by:
1. Catching 429 errors
2. Falling back to alternative providers
3. Returning helpful error messages

## Best Practices

### 1. API Key Security

```bash
# ✅ Good: Use environment variables
DEEPSEEK_API_KEY=sk-xxx

# ❌ Bad: Hardcode in source
const apiKey = "sk-xxx";
```

### 2. Error Handling

```typescript
try {
  const analysis = await requestStrategicAnalysis(context);
  if (!analysis.success) {
    // Handle offline/error state
    showErrorMessage(analysis.reason);
  }
} catch (error) {
  // Handle network errors
  showFallbackUI();
}
```

### 3. Caching

Consider caching AI responses for identical requests:

```typescript
const cacheKey = JSON.stringify(context);
if (cache.has(cacheKey)) {
  return cache.get(cacheKey);
}
const analysis = await requestStrategicAnalysis(context);
cache.set(cacheKey, analysis, { ttl: 300 }); // 5 min cache
```

### 4. Rate Limiting

Implement client-side rate limiting:

```typescript
const rateLimiter = new RateLimiter({ maxRequests: 10, perMinutes: 1 });
await rateLimiter.wait();
const analysis = await requestStrategicAnalysis(context);
```

## Monitoring

### Metrics to Track

1. **Response Time**: Average time for AI analysis
2. **Success Rate**: Percentage of successful requests
3. **Provider Usage**: Distribution across providers
4. **Confidence Scores**: Average confidence over time
5. **Error Rate**: Failed requests by error type

### Logging

```javascript
console.log(`🧠 AI Reasoner: Processed ${results.length} valid results`);
console.log(`🤖 AI Reasoner: Single provider mode (${provider})`);
console.log(`🚀 AI Reasoner: Entering parallel consensus mode...`);
console.log(`✅ AI Reasoner: Merged result ready`);
```

## Troubleshooting

### Issue: "API Key Missing"

**Solution**: Add API key to `.env` file and restart services

### Issue: "All providers failed"

**Possible causes**:
- Invalid API keys
- Network connectivity issues
- Rate limiting
- API service outage

**Solution**: Check logs for specific error messages

### Issue: Low confidence scores

**Possible causes**:
- Insufficient context data
- Ambiguous security telemetry
- Conflicting AI responses

**Solution**: Provide more detailed context in requests

## Future Enhancements

1. **Streaming Responses**: Real-time token streaming
2. **Custom Models**: Fine-tuned models for security
3. **Local LLMs**: Privacy-focused local inference
4. **Advanced Caching**: Semantic similarity caching
5. **A/B Testing**: Compare provider performance
