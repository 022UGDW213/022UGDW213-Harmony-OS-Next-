const express = require('express');
const cors = require('cors');
const { generateStrategicAnalysis, analyzeSpecificThreat } = require('./ai-reasoner.cjs');
const { getThreatIntelligence, extractPatternsFromCVEs } = require('./intelligence-node.cjs');
const path = require('path');

const app = express();
const PORT = process.env.REASONING_PORT || 3003;

app.use(cors());
app.use(express.json());

// Helper to call generic AI Provider
async function callProvider(provider, message, context, apiKey, systemPromptOverride = null) {
    const axios = require('axios');
    let apiUrl, model;

    // Define QBM Persona for High-Level Reasoning
    const QBM_PERSONA = `You are a cybersecurity expert operating as a Quantum Binary Matrix (QBM) state engine.
Your goal is to provide stable, high-coherence security decisions.
If requested, output strictly in the QBM JSON format:
{
  "resonance": { "paths": ["reasoning path 1", "path 2"], "coherence": 0.0-1.0 },
  "qbm": { "activeLayers": ["layer1"], "collapseLayer": 0, "decision": "final actionable summary" },
  "action": { "command": "suggested command", "confidence": 0.0-1.0 }
}
Otherwise, provide clear, strategic guidance.`;

    const SYSTEM_PROMPT = systemPromptOverride || QBM_PERSONA;

    if (provider === 'deepseek') {
        apiUrl = 'https://api.deepseek.com/chat/completions';
        model = 'deepseek-chat';
    } else if (provider === 'nova') {
        apiUrl = process.env.NOVA_API_URL || 'https://api.nova.amazon.com/v1/chat/completions';
        model = 'nova-pro';
    } else {
        apiUrl = 'https://api.moonshot.cn/v1/chat/completions';
        model = 'moonshot-v1-8k';
    }

    try {
        const response = await axios.post(
            apiUrl,
            {
                model: model,
                messages: [{ role: 'system', content: SYSTEM_PROMPT }, ...context, { role: 'user', content: message }],
                max_tokens: 2000,
                temperature: 0.7
            },
            {
                headers: {
                    'Authorization': `Bearer ${apiKey}`,
                    'Content-Type': 'application/json'
                },
                timeout: 30000
            }
        );
        return {
            provider,
            success: true,
            response: response.data.choices[0].message.content,
            model: response.data.model
        };
    } catch (error) {
        return {
            provider,
            success: false,
            error: error.message
        };
    }
}

// Chat endpoint (Parallel Integration)
app.post('/chat', async (req, res) => {
    try {
        const { message, context = [], parallel = false } = req.body;

        const deepseekKey = process.env.DEEPSEEK_API_KEY;
        const novaKey = process.env.NOVA_API_KEY;

        if (!deepseekKey && !novaKey) {
            return res.status(401).json({
                success: false,
                error: 'No AI Keys',
                response: 'Configuration Error: No active AI API keys found.'
            });
        }

        // Parallel Execution Mode
        if (parallel && deepseekKey && novaKey) {
            console.log('🚀 Executing Parallel AI Reasoning (DeepSeek + Nova)...');

            const results = await Promise.all([
                callProvider('deepseek', message, context, deepseekKey),
                callProvider('nova', message, context, novaKey)
            ]);

            const deepseekRes = results.find(r => r.provider === 'deepseek');
            const novaRes = results.find(r => r.provider === 'nova');

            let combinedResponse = `🧠 **DeepSeek Logic:**\n${deepseekRes.success ? deepseekRes.response : 'Failed to connect.'}\n\n`;
            combinedResponse += `💫 **Nova Insight:**\n${novaRes.success ? novaRes.response : 'Failed to connect.'}`;

            return res.json({
                success: true,
                response: combinedResponse, // Human-readable for Chat UI
                results: { // Machine-readable for Smart Nodes
                    deepseek: deepseekRes,
                    nova: novaRes
                },
                mode: 'parallel',
                providers: ['deepseek', 'nova']
            });
        }

        // Single Provider Fallback
        const activeKey = deepseekKey || novaKey;
        const provider = deepseekKey ? 'deepseek' : 'nova';

        const result = await callProvider(provider, message, context, activeKey);

        if (result.success) {
            res.json({
                success: true,
                response: result.response,
                model: result.model
            });
        } else {
            throw new Error(result.error);
        }

    } catch (error) {
        console.error('Chat API error:', error.message);
        res.status(500).json({
            success: false,
            error: error.message,
            response: 'System Error: Unable to connect to AI reasoning service.'
        });
    }
});

// Health check endpoint
app.get('/health', (req, res) => {
    const hasDeepSeekKey = !!process.env.DEEPSEEK_API_KEY;
    const hasNovaKey = !!process.env.NOVA_API_KEY;
    const hasKimiKey = !!process.env.KIMI_API_KEY;

    res.json({
        status: 'online',
        timestamp: new Date().toISOString(),
        aiCapabilities: {
            deepseek: hasDeepSeekKey,
            nova: hasNovaKey,
            kimi: hasKimiKey,
            reasoning: hasDeepSeekKey || hasNovaKey || hasKimiKey,
            parallelMode: hasDeepSeekKey && hasNovaKey
        },
        version: '2.1.0'
    });
});

// Strategic analysis endpoint
app.post('/analyze', async (req, res) => {
    try {
        const { scanResult, brainInsight, learnedPatterns, cveData } = req.body;

        const projectRoot = path.resolve(__dirname, '..');
        const threatIntel = getThreatIntelligence(projectRoot);

        const context = {
            scanResult,
            brainInsight,
            learnedPatterns,
            cveData,
            threatIntelligence: threatIntel
        };

        const analysis = await generateStrategicAnalysis(context);

        res.json({
            success: true,
            analysis,
            metadata: {
                timestamp: new Date().toISOString(),
                contextSize: JSON.stringify(context).length,
                threatIntelAvailable: !!threatIntel
            }
        });
    } catch (error) {
        console.error('Analysis error:', error);
        res.status(500).json({
            success: false,
            error: error.message,
            fallback: {
                strategicBriefing: 'Analysis temporarily unavailable. System operating in fallback mode.',
                riskAssessment: 'MEDIUM',
                recommendations: [
                    'Continue monitoring security events',
                    'Review system logs for anomalies',
                    'Ensure all security patches are applied'
                ]
            }
        });
    }
});

// Capabilities endpoint
app.get('/capabilities', (req, res) => {
    const hasDeepSeek = !!process.env.DEEPSEEK_API_KEY;
    const hasNova = !!process.env.NOVA_API_KEY;

    const capabilities = {
        aiReasoning: hasDeepSeek || hasNova,
        fallbackReasoning: !!process.env.KIMI_API_KEY,
        parallelMode: hasDeepSeek && hasNova,
        threatIntelligence: true,
        patternAnalysis: true,
        strategicAnalysis: true,
        features: [
            'Strategic Security Analysis',
            'Threat Pattern Recognition',
            'Risk Assessment',
            'Predictive Threat Modeling',
            'Automated Recommendations',
            hasDeepSeek && hasNova ? 'Parallel Multi-Model Consensus' : null
        ].filter(Boolean)
    };

    res.json({
        success: true,
        capabilities,
        status: capabilities.aiReasoning ? 'fully-operational' : 'limited-mode'
    });
});

// Kernel Bridge Endpoint
app.get('/api/kernel/status', (req, res) => {
    const fs = require('fs');
    const STATS_FILE = '/tmp/harmony_kernel_stats.json';

    if (fs.existsSync(STATS_FILE)) {
        try {
            const data = fs.readFileSync(STATS_FILE, 'utf8');
            const stats = JSON.parse(data);
            res.json({
                success: true,
                online: true,
                stats: stats
            });
        } catch (e) {
            res.json({ success: false, online: false, error: 'Read Error' });
        }
    } else {
        res.json({
            success: true,
            online: false, // Kernel not running or hasn't written stats yet
            message: 'Kernel bridge file not found'
        });
    }
});

// Start server
app.listen(PORT, () => {
    console.log(`🧠 CyberLab Reasoning API running on port ${PORT}`);
    console.log(`🔗 Health check: http://localhost:${PORT}/health`);

    const hasAI = process.env.DEEPSEEK_API_KEY || process.env.KIMI_API_KEY;
    console.log(`🤖 AI Reasoning: ${hasAI ? '✅ ENABLED' : '❌ DISABLED (API key required)'}`);

    if (hasAI) {
        console.log('🚀 Advanced AI reasoning capabilities are active');
    } else {
        console.log('⚠️  Add DEEPSEEK_API_KEY to .env for full AI capabilities');
    }
});

module.exports = app;