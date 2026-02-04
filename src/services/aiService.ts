import axios from 'axios';

const REASONING_API_URL = 'http://localhost:3001';

export interface StrategicAnalysis {
    success: boolean;
    status: string;
    provider?: string;
    modelType?: string;
    strategicBriefing: string;
    riskAssessment?: string;
    riskReasoning?: string;
    recommendations: string[];
    threatPredictions?: string[];
    confidence: number;
    keyInsights?: string[];
    timestamp?: string;
    reason?: string;
}

export interface AnalysisContext {
    scanResult?: any;
    brainInsight?: any;
    learnedPatterns?: any[];
    cveData?: any[];
}

/**
 * Request strategic analysis from AI reasoning API
 */
export async function requestStrategicAnalysis(
    context: AnalysisContext
): Promise<StrategicAnalysis> {
    try {
        const response = await axios.post<StrategicAnalysis>(
            `${REASONING_API_URL}/api/reasoning/analyze`,
            context,
            {
                timeout: 30000,
                headers: {
                    'Content-Type': 'application/json',
                },
            }
        );
        return response.data;
    } catch (error: any) {
        console.error('AI Reasoning API Error:', error.message);
        return {
            success: false,
            status: 'error',
            reason: error.message || 'Failed to connect to reasoning API',
            strategicBriefing: 'Unable to connect to AI Reasoning API. Please ensure the reasoning server is running.',
            recommendations: [
                'Start the reasoning API with: npm run reasoning-api',
                'Check that port 3001 is not blocked',
                'Verify API keys are configured in .env file',
            ],
            confidence: 0,
        };
    }
}

/**
 * Analyze a specific threat
 */
export async function analyzeSpecificThreat(threatData: any): Promise<any> {
    try {
        const response = await axios.post(
            `${REASONING_API_URL}/api/reasoning/threat`,
            { threatData },
            {
                timeout: 20000,
                headers: {
                    'Content-Type': 'application/json',
                },
            }
        );
        return response.data;
    } catch (error: any) {
        console.error('Threat Analysis Error:', error.message);
        return {
            success: false,
            reason: error.message,
        };
    }
}

/**
 * Check if reasoning API is available
 */
export async function checkReasoningAPIHealth(): Promise<boolean> {
    try {
        const response = await axios.get(`${REASONING_API_URL}/health`, {
            timeout: 3000,
        });
        return response.status === 200;
    } catch {
        return false;
    }
}
