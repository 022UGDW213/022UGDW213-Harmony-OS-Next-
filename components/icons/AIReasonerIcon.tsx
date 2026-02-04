export default function AIReasonerIcon() {
    return (
        <svg viewBox="0 0 64 64" fill="none" xmlns="http://www.w3.org/2000/svg">
            <defs>
                <linearGradient id="aiGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                    <stop offset="0%" stopColor="#8B5CF6" />
                    <stop offset="100%" stopColor="#EC4899" />
                </linearGradient>
            </defs>
            <rect width="64" height="64" rx="14" fill="url(#aiGrad)" />
            <circle cx="32" cy="28" r="12" fill="white" opacity="0.9" />
            <path d="M20 44 L32 38 L44 44" stroke="white" strokeWidth="3" strokeLinecap="round" opacity="0.9" />
            <circle cx="24" cy="28" r="2" fill="#8B5CF6" />
            <circle cx="32" cy="24" r="2" fill="#8B5CF6" />
            <circle cx="40" cy="28" r="2" fill="#8B5CF6" />
        </svg>
    );
}
