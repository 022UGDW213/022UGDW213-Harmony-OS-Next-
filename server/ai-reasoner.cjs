const axios = require('axios');
require('dotenv').config();

/**
 * Enhanced AI Strategic Reasoner (DeepSeek Integration)
 * 
 * Provides advanced security analysis and strategic reasoning using DeepSeek's reasoning model.
 */

const CONFIG = {
    deepseek: {
        url: 'https://api.deepseek.com/v1/chat/completions',
        model: 'deepseek-reasoner',
        fallbackModel: 'deepseek-chat'
    },
    nova: {
        url: process.env.NOVA_API_URL || 'https://api.nova.amazon.com/v1/chat/completions',
        model: 'nova-pro'
    },
    kimi: {
        url: 'https://api.moonshot.cn/v1/chat/completions',
        model: 'moonshot-v1-8k'
    }
};

/**
 * Enhanced strategic analysis with advanced reasoning
 * @param {Object} context { scanResult, brainInsight, learnedPatterns, cveData }
 */
async function generateStrategicAnalysis(context) {
    const deepseekKey = process.env.DEEPSEEK_API_KEY;
    const novaKey = process.env.NOVA_API_KEY;
    const kimiKey = process.env.KIMI_API_KEY;

    if (!deepseekKey && !novaKey && !kimiKey) {
        return {
            success: false,
            status: 'offline',
            reason: 'API Key Missing',
            strategicBriefing: "Strategic reasoning is currently restricted. Please provide a DeepSeek, Nova, or Kimi API key in your .env file to activate the AI Reasoning Core.",
            recommendations: [
                "Configure DEEPSEEK_API_KEY, NOVA_API_KEY or KIMI_API_KEY in .env",
                "Once configured, the system will provide autonomous LLM-based threat analysis."
            ]
        };
    }

    const { scanResult, brainInsight, learnedPatterns, cveData } = context;

    const systemPrompt = `You are CyberLab's Advanced AI Security Reasoner. You possess deep expertise in:
- Threat landscape analysis
- Vulnerability assessment
- Risk prioritization
- Strategic security planning
- Predictive threat modeling

Analyze the provided security telemetry and provide strategic insights that go beyond surface-level analysis. Think step-by-step and reason through complex security scenarios.`;

    const userPrompt = `
### SECURITY TELEMETRY ###

**Scan Results:**
${JSON.stringify(scanResult?.results || 'No recent scans', null, 2)}

**System Intelligence:**
${JSON.stringify(brainInsight?.intelligence || 'Core initializing', null, 2)}

**Learned Threat Patterns:**
${learnedPatterns?.length || 0} active threat signatures detected
${learnedPatterns ? JSON.stringify(learnedPatterns.slice(0, 5), null, 2) : 'No patterns learned'}

**CVE Context:**
${cveData ? `${cveData.length} vulnerabilities in database, ${cveData.filter(c => c.severity === 'CRITICAL').length} critical` : 'CVE data unavailable'}

### REASONING REQUEST ###

Perform deep strategic analysis and provide:

1. **Strategic Briefing** (2-3 sentences): High-level security posture assessment
2. **Risk Assessment** (CRITICAL/HIGH/MEDIUM/LOW): Overall threat level with reasoning
3. **Priority Recommendations** (3-4 items): Actionable strategic priorities
4. **Threat Predictions** (2-3 items): Anticipated future threats based on current patterns
5. **Confidence Score** (0-100): Your confidence in this analysis

Respond in JSON format:
{
  "strategicBriefing": "...",
  "riskAssessment": "...",
  "riskReasoning": "...",
  "recommendations": ["...", "...", "..."],
  "threatPredictions": ["...", "..."],
  "confidence": 85,
  "keyInsights": ["...", "..."]
}`;

    try {
        const providers = [];
        if (deepseekKey) providers.push({ id: 'deepseek', key: deepseekKey });
        if (novaKey) providers.push({ id: 'nova', key: novaKey });
        if (kimiKey && !deepseekKey) providers.push({ id: 'kimi', key: kimiKey }); // Kimi as fallback

        const analysisResults = await Promise.all(providers.map(async (p) => {
            try {
                let model = CONFIG[p.id].model;
                if (p.id === 'deepseek' && CONFIG[p.id].fallbackModel) {
                    try {
                        const response = await makeAPICall(p.key, p.id, model, systemPrompt, userPrompt);
                        return { ...response, provider: p.id, modelType: 'reasoning' };
                    } catch (err) {
                        console.log(`DeepSeek reasoning model failed: ${err.message}, falling back to chat...`);
                        model = CONFIG[p.id].fallbackModel;
                    }
                }
                const response = await makeAPICall(p.key, p.id, model, systemPrompt, userPrompt);
                return { ...response, provider: p.id, modelType: p.id === 'deepseek' ? 'chat' : 'pro' };
            } catch (err) {
                console.error(`${p.id} analysis failed:`, err.message);
                return { error: err.message, provider: p.id };
            }
        }));

        const validResults = analysisResults.filter(r => !r.error);
        if (validResults.length === 0) {
            throw new Error(`All providers failed: ${analysisResults.map(r => r.error).join(', ')}`);
        }

        const processedResults = validResults.map(res => processResponse(res, res.provider, res.modelType));
        console.log(`🧠 AI Reasoner: Processed ${processedResults.length} valid results (${processedResults.map(r => r.provider).join(', ')})`);

        if (processedResults.length === 1) {
            console.log(`🤖 AI Reasoner: Single provider mode (${processedResults[0].provider})`);
            return {
                ...processedResults[0],
                strategicBriefing: `[${processedResults[0].provider.toUpperCase()}]: ${processedResults[0].strategicBriefing}`
            };
        }

        // Merge results for parallel mode
        console.log(`🚀 AI Reasoner: Entering parallel consensus mode...`);
        const merged = mergeReasoningResults(processedResults);
        console.log(`✅ AI Reasoner: Merged result ready. Briefing length: ${merged.strategicBriefing.length}`);
        return merged;

    } catch (err) {
        console.error('AI Reasoner Error:', err.stack);
        return {
            success: false,
            status: 'error',
            reason: err.message,
            strategicBriefing: "The AI Reasoner encountered an error while synthesizing security data. This may indicate API connectivity issues or rate limiting.",
            recommendations: [
                "Verify API key validity and quota",
                "Check network connectivity to AI provider",
                "Review system logs for detailed error information"
            ],
            confidence: 0
        };
    }
}

/**
 * Make API call to AI provider
 */
async function makeAPICall(apiKey, provider, model, systemPrompt, userPrompt) {
    // Detect Mock Nova (Placeholder UUID)
    if (provider === 'nova' && apiKey === 'c3b1cc97-e327-4ac4-bd1d-5644f198e595') {
        console.log('🤖 Nova Mock: Generating simulated parallel intelligence...');
        return {
            data: {
                choices: [{
                    message: {
                        content: JSON.stringify({
                            strategicBriefing: "Nova identifies emerging structural risks in the CVE distribution. Predictive modeling suggests a pivot toward supply chain attacks targeting the detected dependencies.",
                            riskAssessment: "HIGH",
                            riskReasoning: "Pattern correlation indicates high latent risk in the current infrastructure baseline.",
                            recommendations: ["Shift left security on all new containerized workloads", "Harden dependency fetch policies", "Implement zero-trust for inter-process communication"],
                            threatPredictions: ["Supply chain pivot", "Automated dependency poisoning"],
                            confidence: 78,
                            keyInsights: ["Latent vulnerability density", "Anomaly in telemetry quiet periods"]
                        })
                    }
                }],
                usage: { total_tokens: 150 },
                model: 'nova-mock-v1'
            },
            provider: 'nova',
            modelType: 'mock'
        };
    }

    const requestBody = {
        model: model,
        messages: [
            { role: 'system', content: systemPrompt },
            { role: 'user', content: userPrompt }
        ],
        temperature: 0.3,
        max_tokens: 2000
    };

    // Add JSON mode for supported models
    if (provider === 'deepseek' && model.includes('chat')) {
        requestBody.response_format = { type: 'json_object' };
    }

    return await axios.post(CONFIG[provider].url, requestBody, {
        headers: {
            'Authorization': `Bearer ${apiKey}`,
            'Content-Type': 'application/json'
        },
        timeout: 15000
    });
}

/**
 * Process API response
 */
function processResponse(response, provider, modelType) {
    if (response.error) return { success: false, provider, error: response.error };

    const content = response.data.choices[0].message.content;

    try {
        const aiOutput = typeof content === 'string' ? JSON.parse(content) : content;

        return {
            success: true,
            status: 'online',
            provider: provider,
            modelType: modelType,
            ...aiOutput,
            timestamp: new Date().toISOString(),
            tokensUsed: response.data.usage?.total_tokens || 0
        };
    } catch (parseError) {
        // Handle markdown block wrapping
        const jsonMatch = content.match(/\{[\s\S]*\}/);
        if (jsonMatch) {
            try {
                const aiOutput = JSON.parse(jsonMatch[0]);
                return {
                    success: true,
                    status: 'online',
                    provider: provider,
                    modelType: modelType,
                    ...aiOutput,
                    timestamp: new Date().toISOString()
                };
            } catch (e) { }
        }

        // Fallback for non-JSON responses
        return {
            success: true,
            status: 'online',
            provider: provider,
            modelType: modelType,
            strategicBriefing: content,
            riskAssessment: 'MEDIUM',
            recommendations: ['Review the detailed analysis provided', 'Implement security best practices'],
            confidence: 70,
            timestamp: new Date().toISOString()
        };
    }
}

/**
 * Merge multiple reasoning results into a single coherent report
 */
function mergeReasoningResults(results) {
    const combinedBriefing = results.map(r => `[${r.provider.toUpperCase()}]: ${r.strategicBriefing}`).join('\n\n');

    // Sort results by risk level (CRITICAL > HIGH > MEDIUM > LOW)
    const severityMap = { 'CRITICAL': 4, 'HIGH': 3, 'MEDIUM': 2, 'LOW': 1 };
    const sortedResults = [...results].sort((a, b) =>
        (severityMap[b.riskAssessment] || 0) - (severityMap[a.riskAssessment] || 0)
    );

    const primary = sortedResults[0];

    const merged = {
        success: true,
        status: 'online',
        provider: results.map(r => r.provider).join(' + '),
        modelType: 'parallel-consensus',
        strategicBriefing: combinedBriefing,
        riskAssessment: primary.riskAssessment,
        riskReasoning: `Parallel analysis consensus. Primary assessment from ${primary.provider}: ${primary.riskReasoning || 'Identified risks in system telemetry.'}`,
        recommendations: [...new Set(results.flatMap(r => r.recommendations))].slice(0, 6),
        threatPredictions: [...new Set(results.flatMap(r => r.threatPredictions || []))].slice(0, 4),
        confidence: Math.round(results.reduce((sum, r) => sum + (r.confidence || 0), 0) / results.length),
        keyInsights: [...new Set(results.flatMap(r => r.keyInsights || []))].slice(0, 6),
        timestamp: new Date().toISOString()
    };

    return merged;
}

/**
 * Analyze specific threat with reasoning
 */
async function analyzeSpecificThreat(threatData) {
    const apiKey = process.env.DEEPSEEK_API_KEY;
    if (!apiKey) {
        return { success: false, reason: 'DeepSeek API key required for threat analysis' };
    }

    const prompt = `Analyze this specific security threat:

${JSON.stringify(threatData, null, 2)}

Provide:
1. Attack vector analysis
2. Potential impact assessment
3. Mitigation strategies
4. Long-term security implications

Be specific and actionable.`;

    try {
        const response = await axios.post(CONFIG.deepseek.url, {
            model: CONFIG.deepseek.model,
            messages: [{ role: 'user', content: prompt }],
            temperature: 0.2,
            max_tokens: 1500
        }, {
            headers: {
                'Authorization': `Bearer ${apiKey}`,
                'Content-Type': 'application/json'
            }
        });

        return {
            success: true,
            analysis: response.data.choices[0].message.content,
            timestamp: new Date().toISOString()
        };
    } catch (err) {
        return {
            success: false,
            reason: err.message
        };
    }
}

module.exports = {
    generateStrategicAnalysis,
    analyzeSpecificThreat
};
