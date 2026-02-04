import React, { useState, useEffect } from 'react';
import { requestStrategicAnalysis, checkReasoningAPIHealth, type StrategicAnalysis } from '../services/aiService';

export default function AIReasonerContent() {
    const [analysis, setAnalysis] = useState<StrategicAnalysis | null>(null);
    const [loading, setLoading] = useState(false);
    const [apiOnline, setApiOnline] = useState(false);

    useEffect(() => {
        checkAPIStatus();
        const interval = setInterval(checkAPIStatus, 10000);
        return () => clearInterval(interval);
    }, []);

    const checkAPIStatus = async () => {
        const online = await checkReasoningAPIHealth();
        setApiOnline(online);
    };

    const performAnalysis = async () => {
        setLoading(true);
        try {
            const result = await requestStrategicAnalysis({
                scanResult: { status: 'healthy' },
                brainInsight: { intelligence: ['System operational'] },
            });
            setAnalysis(result);
        } catch (error) {
            console.error('Analysis failed:', error);
        } finally {
            setLoading(false);
        }
    };

    const getRiskColor = (risk?: string) => {
        switch (risk) {
            case 'CRITICAL': return 'text-red-500';
            case 'HIGH': return 'text-orange-500';
            case 'MEDIUM': return 'text-yellow-500';
            case 'LOW': return 'text-green-500';
            default: return 'text-gray-400';
        }
    };

    const getConfidenceColor = (confidence: number) => {
        if (confidence >= 80) return 'bg-green-500';
        if (confidence >= 60) return 'bg-yellow-500';
        if (confidence >= 40) return 'bg-orange-500';
        return 'bg-red-500';
    };

    return (
        <div className="p-4 text-white h-full overflow-y-auto flex flex-col gap-4">
            <div className="flex items-center justify-between">
                <h2 className="text-lg font-semibold">AI Strategic Reasoner</h2>
                <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${apiOnline ? 'bg-green-500' : 'bg-red-500'}`} />
                    <span className="text-xs opacity-70">{apiOnline ? 'Online' : 'Offline'}</span>
                </div>
            </div>

            <button
                onClick={performAnalysis}
                disabled={loading || !apiOnline}
                className="w-full bg-gradient-to-r from-purple-600 to-pink-600 hover:from-purple-700 hover:to-pink-700 disabled:opacity-50 disabled:cursor-not-allowed text-white font-bold py-3 px-4 rounded-lg transition-all"
            >
                {loading ? 'Analyzing...' : 'Request Strategic Analysis'}
            </button>

            {!apiOnline && (
                <div className="bg-red-500/20 border border-red-500/50 rounded-lg p-3 text-sm">
                    <p className="font-semibold mb-1">⚠️ Reasoning API Offline</p>
                    <p className="text-xs opacity-80">Start the API with: <code className="bg-black/30 px-1 rounded">npm run reasoning-api</code></p>
                </div>
            )}

            {analysis && (
                <div className="flex-grow space-y-3">
                    {/* Provider Info */}
                    {analysis.provider && (
                        <div className="bg-white/5 rounded-lg p-2 text-xs">
                            <span className="opacity-70">Provider:</span> <span className="font-semibold">{analysis.provider}</span>
                            {analysis.modelType && <span className="opacity-70 ml-2">({analysis.modelType})</span>}
                        </div>
                    )}

                    {/* Strategic Briefing */}
                    <div className="bg-white/10 rounded-lg p-3">
                        <h3 className="text-sm font-semibold mb-2">📊 Strategic Briefing</h3>
                        <p className="text-sm opacity-90 whitespace-pre-wrap">{analysis.strategicBriefing}</p>
                    </div>

                    {/* Risk Assessment */}
                    {analysis.riskAssessment && (
                        <div className="bg-white/10 rounded-lg p-3">
                            <h3 className="text-sm font-semibold mb-2">🎯 Risk Assessment</h3>
                            <div className="flex items-center gap-2">
                                <span className={`text-lg font-bold ${getRiskColor(analysis.riskAssessment)}`}>
                                    {analysis.riskAssessment}
                                </span>
                                {analysis.confidence > 0 && (
                                    <div className="flex-grow">
                                        <div className="flex items-center gap-2">
                                            <div className="flex-grow bg-white/20 rounded-full h-2">
                                                <div
                                                    className={`h-2 rounded-full ${getConfidenceColor(analysis.confidence)}`}
                                                    style={{ width: `${analysis.confidence}%` }}
                                                />
                                            </div>
                                            <span className="text-xs opacity-70">{analysis.confidence}%</span>
                                        </div>
                                    </div>
                                )}
                            </div>
                            {analysis.riskReasoning && (
                                <p className="text-xs opacity-80 mt-2">{analysis.riskReasoning}</p>
                            )}
                        </div>
                    )}

                    {/* Recommendations */}
                    {analysis.recommendations && analysis.recommendations.length > 0 && (
                        <div className="bg-white/10 rounded-lg p-3">
                            <h3 className="text-sm font-semibold mb-2">💡 Recommendations</h3>
                            <ul className="space-y-1">
                                {analysis.recommendations.map((rec, idx) => (
                                    <li key={idx} className="text-xs opacity-90 flex gap-2">
                                        <span className="opacity-50">•</span>
                                        <span>{rec}</span>
                                    </li>
                                ))}
                            </ul>
                        </div>
                    )}

                    {/* Threat Predictions */}
                    {analysis.threatPredictions && analysis.threatPredictions.length > 0 && (
                        <div className="bg-white/10 rounded-lg p-3">
                            <h3 className="text-sm font-semibold mb-2">🔮 Threat Predictions</h3>
                            <ul className="space-y-1">
                                {analysis.threatPredictions.map((threat, idx) => (
                                    <li key={idx} className="text-xs opacity-90 flex gap-2">
                                        <span className="opacity-50">•</span>
                                        <span>{threat}</span>
                                    </li>
                                ))}
                            </ul>
                        </div>
                    )}

                    {/* Key Insights */}
                    {analysis.keyInsights && analysis.keyInsights.length > 0 && (
                        <div className="bg-white/10 rounded-lg p-3">
                            <h3 className="text-sm font-semibold mb-2">🔍 Key Insights</h3>
                            <div className="flex flex-wrap gap-2">
                                {analysis.keyInsights.map((insight, idx) => (
                                    <span key={idx} className="text-xs bg-white/20 px-2 py-1 rounded">
                                        {insight}
                                    </span>
                                ))}
                            </div>
                        </div>
                    )}

                    {/* Timestamp */}
                    {analysis.timestamp && (
                        <div className="text-xs opacity-50 text-center">
                            Last updated: {new Date(analysis.timestamp).toLocaleTimeString()}
                        </div>
                    )}
                </div>
            )}
        </div>
    );
}
