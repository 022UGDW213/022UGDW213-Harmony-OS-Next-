import React, { useState, useEffect } from 'react';

interface BrainMetrics {
    status: string;
    metrics: {
        ramUsage: string;
        activeTasks: number;
    };
    intelligence: string[];
    timestamp: string;
}

export default function SystemMonitorContent() {
    const [brainData, setBrainData] = useState<BrainMetrics | null>(null);
    const [connected, setConnected] = useState(false);

    useEffect(() => {
        // Simulate brain.cpp data stream
        // In production, this would read from the brain output or via WebSocket
        const interval = setInterval(() => {
            const mockData: BrainMetrics = {
                status: 'conscious',
                metrics: {
                    ramUsage: `${Math.floor(Math.random() * 8000 + 24000)} MB`,
                    activeTasks: Math.floor(Math.random() * 50 + 200),
                },
                intelligence: [
                    'Brain Insight: System baseline stable. No anomalous shifts in security posture.',
                    'Performance: Optimal resource allocation detected.',
                    'Network: All communication channels secure.',
                ],
                timestamp: new Date().toISOString(),
            };
            setBrainData(mockData);
            setConnected(true);
        }, 2000);

        return () => clearInterval(interval);
    }, []);

    return (
        <div className="p-4 text-white h-full overflow-y-auto flex flex-col gap-4">
            <div className="flex items-center justify-between">
                <h2 className="text-lg font-semibold">System Monitor</h2>
                <div className="flex items-center gap-2">
                    <div className={`w-2 h-2 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
                    <span className="text-xs opacity-70">{connected ? 'Brain Core Active' : 'Disconnected'}</span>
                </div>
            </div>

            {!connected && (
                <div className="bg-yellow-500/20 border border-yellow-500/50 rounded-lg p-3 text-sm">
                    <p className="font-semibold mb-1">⚠️ Brain Core Offline</p>
                    <p className="text-xs opacity-80">Start with: <code className="bg-black/30 px-1 rounded">npm run dev:ai</code></p>
                </div>
            )}

            {brainData && (
                <div className="flex-grow space-y-3">
                    {/* Status */}
                    <div className="bg-white/10 rounded-lg p-3">
                        <h3 className="text-sm font-semibold mb-2">🧠 Core Status</h3>
                        <div className="flex items-center gap-2">
                            <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse" />
                            <span className="text-sm capitalize">{brainData.status}</span>
                        </div>
                    </div>

                    {/* System Metrics */}
                    <div className="bg-white/10 rounded-lg p-3">
                        <h3 className="text-sm font-semibold mb-3">📊 System Metrics</h3>
                        <div className="grid grid-cols-2 gap-3">
                            <div className="bg-white/5 rounded p-2">
                                <div className="text-xs opacity-70 mb-1">RAM Usage</div>
                                <div className="text-lg font-mono font-bold">{brainData.metrics.ramUsage}</div>
                            </div>
                            <div className="bg-white/5 rounded p-2">
                                <div className="text-xs opacity-70 mb-1">Active Tasks</div>
                                <div className="text-lg font-mono font-bold">{brainData.metrics.activeTasks}</div>
                            </div>
                        </div>
                    </div>

                    {/* Intelligence Insights */}
                    <div className="bg-white/10 rounded-lg p-3">
                        <h3 className="text-sm font-semibold mb-2">💡 Intelligence Insights</h3>
                        <div className="space-y-2">
                            {brainData.intelligence.map((insight, idx) => (
                                <div key={idx} className="bg-white/5 rounded p-2 text-xs">
                                    <p className="opacity-90">{insight}</p>
                                </div>
                            ))}
                        </div>
                    </div>

                    {/* CPU Usage Visualization */}
                    <div className="bg-white/10 rounded-lg p-3">
                        <h3 className="text-sm font-semibold mb-2">⚡ Resource Monitor</h3>
                        <div className="space-y-2">
                            <div>
                                <div className="flex justify-between text-xs mb-1">
                                    <span>CPU</span>
                                    <span>{Math.floor(Math.random() * 30 + 20)}%</span>
                                </div>
                                <div className="w-full bg-white/20 rounded-full h-2">
                                    <div
                                        className="bg-gradient-to-r from-green-500 to-blue-500 h-2 rounded-full transition-all duration-500"
                                        style={{ width: `${Math.floor(Math.random() * 30 + 20)}%` }}
                                    />
                                </div>
                            </div>
                            <div>
                                <div className="flex justify-between text-xs mb-1">
                                    <span>Memory</span>
                                    <span>{Math.floor(Math.random() * 20 + 60)}%</span>
                                </div>
                                <div className="w-full bg-white/20 rounded-full h-2">
                                    <div
                                        className="bg-gradient-to-r from-blue-500 to-purple-500 h-2 rounded-full transition-all duration-500"
                                        style={{ width: `${Math.floor(Math.random() * 20 + 60)}%` }}
                                    />
                                </div>
                            </div>
                            <div>
                                <div className="flex justify-between text-xs mb-1">
                                    <span>Network</span>
                                    <span>{Math.floor(Math.random() * 15 + 10)}%</span>
                                </div>
                                <div className="w-full bg-white/20 rounded-full h-2">
                                    <div
                                        className="bg-gradient-to-r from-purple-500 to-pink-500 h-2 rounded-full transition-all duration-500"
                                        style={{ width: `${Math.floor(Math.random() * 15 + 10)}%` }}
                                    />
                                </div>
                            </div>
                        </div>
                    </div>

                    {/* Timestamp */}
                    <div className="text-xs opacity-50 text-center">
                        Last update: {new Date(brainData.timestamp).toLocaleTimeString()}
                    </div>
                </div>
            )}
        </div>
    );
}
