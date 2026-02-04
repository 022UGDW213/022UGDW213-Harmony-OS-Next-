export default function SystemMonitorIcon() {
    return (
        <svg viewBox="0 0 64 64" fill="none" xmlns="http://www.w3.org/2000/svg">
            <defs>
                <linearGradient id="monitorGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                    <stop offset="0%" stopColor="#10B981" />
                    <stop offset="100%" stopColor="#3B82F6" />
                </linearGradient>
            </defs>
            <rect width="64" height="64" rx="14" fill="url(#monitorGrad)" />
            <rect x="12" y="16" width="40" height="28" rx="2" fill="white" opacity="0.2" />
            <path d="M16 36 L22 30 L28 34 L34 26 L40 32 L46 28" stroke="white" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round" opacity="0.9" />
            <rect x="16" y="48" width="8" height="4" rx="1" fill="white" opacity="0.7" />
            <rect x="28" y="48" width="8" height="4" rx="1" fill="white" opacity="0.7" />
            <rect x="40" y="48" width="8" height="4" rx="1" fill="white" opacity="0.7" />
        </svg>
    );
}
