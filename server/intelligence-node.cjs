const fs = require('fs');
const path = require('path');

/**
 * Enhanced CVE Intelligence Node
 * Advanced pattern extraction and threat intelligence learning from CVE database.
 */

const ENHANCED_KEYWORDS = [
    // Code Execution Patterns
    { term: 'eval(', category: 'Code Execution', severity: 'critical', weight: 10 },
    { term: 'atob(', category: 'Obfuscation', severity: 'high', weight: 8 },
    { term: 'Remote Code Execution', category: 'RCE', severity: 'critical', weight: 10 },
    { term: 'Command Injection', category: 'Injection', severity: 'critical', weight: 9 },
    { term: 'arbitrary code', category: 'Code Execution', severity: 'critical', weight: 9 },

    // Injection Attacks
    { term: 'SQL Injection', category: 'SQLi', severity: 'high', weight: 8 },
    { term: 'XSS', category: 'Cross-Site Scripting', severity: 'high', weight: 7 },
    { term: 'LDAP injection', category: 'Injection', severity: 'high', weight: 7 },
    { term: 'NoSQL injection', category: 'Injection', severity: 'high', weight: 7 },

    // Authentication & Authorization
    { term: 'bypass', category: 'Security Bypass', severity: 'high', weight: 8 },
    { term: 'privilege escalation', category: 'Privilege Escalation', severity: 'high', weight: 8 },
    { term: 'authentication bypass', category: 'Auth Bypass', severity: 'critical', weight: 9 },
    { term: 'hardcoded', category: 'Hardcoded Credentials', severity: 'high', weight: 7 },
    { term: 'default password', category: 'Weak Authentication', severity: 'high', weight: 7 },

    // Network & Protocol Attacks
    { term: 'SSRF', category: 'Server-Side Request Forgery', severity: 'high', weight: 8 },
    { term: 'CSRF', category: 'Cross-Site Request Forgery', severity: 'medium', weight: 6 },
    { term: 'man-in-the-middle', category: 'MITM', severity: 'high', weight: 7 },
    { term: 'DNS poisoning', category: 'DNS Attack', severity: 'high', weight: 7 },

    // Cryptographic Issues
    { term: 'weak encryption', category: 'Crypto Weakness', severity: 'high', weight: 7 },
    { term: 'broken cryptography', category: 'Crypto Failure', severity: 'high', weight: 8 },
    { term: 'insecure random', category: 'Weak Randomness', severity: 'medium', weight: 6 },

    // File System & Path Issues
    { term: 'path traversal', category: 'Path Traversal', severity: 'high', weight: 7 },
    { term: 'directory traversal', category: 'Directory Traversal', severity: 'high', weight: 7 },
    { term: 'file inclusion', category: 'File Inclusion', severity: 'high', weight: 7 },

    // Memory & Buffer Issues
    { term: 'buffer overflow', category: 'Buffer Overflow', severity: 'critical', weight: 9 },
    { term: 'heap overflow', category: 'Heap Overflow', severity: 'critical', weight: 9 },
    { term: 'stack overflow', category: 'Stack Overflow', severity: 'high', weight: 8 },
    { term: 'use after free', category: 'Memory Corruption', severity: 'critical', weight: 9 },

    // Denial of Service
    { term: 'denial of service', category: 'DoS', severity: 'medium', weight: 6 },
    { term: 'resource exhaustion', category: 'Resource DoS', severity: 'medium', weight: 6 },
    { term: 'infinite loop', category: 'Logic DoS', severity: 'medium', weight: 5 },

    // Information Disclosure
    { term: 'information disclosure', category: 'Info Disclosure', severity: 'medium', weight: 6 },
    { term: 'sensitive data', category: 'Data Exposure', severity: 'high', weight: 7 },
    { term: 'memory leak', category: 'Memory Disclosure', severity: 'medium', weight: 5 },

    // Advanced Persistent Threats
    { term: 'backdoor', category: 'Backdoor', severity: 'critical', weight: 10 },
    { term: 'rootkit', category: 'Rootkit', severity: 'critical', weight: 10 },
    { term: 'supply chain', category: 'Supply Chain Attack', severity: 'critical', weight: 9 },

    // Zero-day & Advanced Threats
    { term: 'zero-day', category: 'Zero-day', severity: 'critical', weight: 10 },
    { term: 'APT', category: 'Advanced Persistent Threat', severity: 'critical', weight: 9 },
    { term: 'nation-state', category: 'State-sponsored', severity: 'critical', weight: 9 }
];

/**
 * Extract enhanced patterns from CVE database with advanced analysis
 */
function extractPatternsFromCVEs(projectRoot) {
    const cvePath = path.join(projectRoot, 'src/data/cveDatabase.json');
    const intelPath = path.join(projectRoot, 'data/learned-intelligence.json');

    if (!fs.existsSync(cvePath)) {
        throw new Error('CVE database not found');
    }

    const cves = JSON.parse(fs.readFileSync(cvePath, 'utf8'));
    const learnedPatterns = [];
    const threatStats = {
        totalAnalyzed: 0,
        criticalThreats: 0,
        emergingPatterns: [],
        topCategories: new Map()
    };

    // Enhanced filtering: high severity CVEs and recent threats
    const highRiskCVEs = cves.filter(cve => {
        const isCritical = cve.cvssScore >= 7.0 || cve.severity === 'CRITICAL' || cve.severity === 'HIGH';
        const isRecent = new Date(cve.publishedDate) > new Date(Date.now() - 365 * 24 * 60 * 60 * 1000); // Last year
        return isCritical || isRecent;
    });

    threatStats.totalAnalyzed = highRiskCVEs.length;

    highRiskCVEs.forEach(cve => {
        const desc = cve.description.toLowerCase();
        let cveRiskScore = 0;

        ENHANCED_KEYWORDS.forEach(kw => {
            if (desc.includes(kw.term.toLowerCase())) {
                cveRiskScore += kw.weight;

                // Track category frequency
                const count = threatStats.topCategories.get(kw.category) || 0;
                threatStats.topCategories.set(kw.category, count + 1);

                learnedPatterns.push({
                    cveId: cve.id,
                    name: `${kw.category} Pattern from ${cve.id}`,
                    rule: kw.category,
                    term: kw.term,
                    severity: kw.severity,
                    weight: kw.weight,
                    riskScore: cveRiskScore,
                    source: 'Enhanced CVE Intelligence',
                    publishedDate: cve.publishedDate,
                    cvssScore: cve.cvssScore
                });

                if (kw.severity === 'critical') {
                    threatStats.criticalThreats++;
                }
            }
        });

        // Detect emerging patterns (new attack vectors)
        if (cveRiskScore > 15) {
            threatStats.emergingPatterns.push({
                cveId: cve.id,
                riskScore: cveRiskScore,
                description: cve.description.substring(0, 100) + '...'
            });
        }
    });

    // Merge AI-Learned Insights (DeepSeek + Nova)
    try {
        const aiIntelligence = getThreatIntelligence(projectRoot);
        if (aiIntelligence && aiIntelligence.patterns) {
            console.log(`🧠 Merging ${aiIntelligence.patterns.length} AI-Learned patterns into consensus...`);
            aiIntelligence.patterns.forEach(aiPattern => {
                learnedPatterns.push({
                    ...aiPattern,
                    source: 'Parallel AI Consensus',
                    isAI: true
                });
            });
        }
    } catch (err) {
        console.warn('⚠️ AI Intelligence merge skipped: No patterns found.');
    }

    // Advanced deduplication with pattern clustering
    const clusteredPatterns = clusterSimilarPatterns(learnedPatterns);

    // Generate threat intelligence summary
    const intelligenceSummary = {
        generatedAt: new Date().toISOString(),
        stats: {
            ...threatStats,
            topCategories: Array.from(threatStats.topCategories.entries())
                .sort((a, b) => b[1] - a[1])
                .slice(0, 10),
            emergingPatterns: threatStats.emergingPatterns
                .sort((a, b) => b.riskScore - a.riskScore)
                .slice(0, 5)
        },
        patterns: clusteredPatterns,
        riskAssessment: generateRiskAssessment(threatStats)
    };

    // Save enhanced intelligence
    const intelDir = path.dirname(intelPath);
    if (!fs.existsSync(intelDir)) {
        fs.mkdirSync(intelDir, { recursive: true });
    }

    fs.writeFileSync(intelPath, JSON.stringify(intelligenceSummary, null, 2));

    return intelligenceSummary;
}

/**
 * Cluster similar patterns to reduce noise
 */
function clusterSimilarPatterns(patterns) {
    const clusters = new Map();

    patterns.forEach(pattern => {
        const key = `${pattern.rule}-${pattern.term}`;
        if (!clusters.has(key)) {
            clusters.set(key, {
                ...pattern,
                occurrences: 1,
                cveIds: [pattern.cveId],
                avgCvssScore: pattern.cvssScore || 0
            });
        } else {
            const existing = clusters.get(key);
            existing.occurrences++;
            existing.cveIds.push(pattern.cveId);
            existing.avgCvssScore = (existing.avgCvssScore + (pattern.cvssScore || 0)) / 2;
            existing.riskScore = Math.max(existing.riskScore, pattern.riskScore);
        }
    });

    return Array.from(clusters.values())
        .sort((a, b) => b.riskScore - a.riskScore)
        .slice(0, 50); // Top 50 most critical patterns
}

/**
 * Generate risk assessment based on threat statistics
 */
function generateRiskAssessment(stats) {
    const criticalRatio = stats.criticalThreats / stats.totalAnalyzed;
    const emergingThreats = stats.emergingPatterns.length;

    let riskLevel = 'LOW';
    let reasoning = [];

    if (criticalRatio > 0.3) {
        riskLevel = 'CRITICAL';
        reasoning.push(`High critical threat density: ${(criticalRatio * 100).toFixed(1)}%`);
    } else if (criticalRatio > 0.15) {
        riskLevel = 'HIGH';
        reasoning.push(`Elevated critical threats: ${(criticalRatio * 100).toFixed(1)}%`);
    } else if (criticalRatio > 0.05) {
        riskLevel = 'MEDIUM';
        reasoning.push(`Moderate threat level: ${(criticalRatio * 100).toFixed(1)}%`);
    }

    if (emergingThreats > 10) {
        riskLevel = riskLevel === 'LOW' ? 'MEDIUM' : riskLevel;
        reasoning.push(`${emergingThreats} emerging high-risk patterns detected`);
    }

    return {
        level: riskLevel,
        reasoning: reasoning,
        score: Math.min(100, (criticalRatio * 100) + (emergingThreats * 2))
    };
}

/**
 * Load enhanced learned patterns for the Malware Scanner
 */
function loadLearnedPatterns(projectRoot) {
    const intelPath = path.join(projectRoot, 'data/learned-intelligence.json');
    if (fs.existsSync(intelPath)) {
        try {
            const intelligence = JSON.parse(fs.readFileSync(intelPath, 'utf8'));
            return intelligence.patterns || [];
        } catch (err) {
            console.error('Error loading learned patterns:', err.message);
            return [];
        }
    }
    return [];
}

/**
 * Get threat intelligence summary
 */
function getThreatIntelligence(projectRoot) {
    const intelPath = path.join(projectRoot, 'data/learned-intelligence.json');
    if (fs.existsSync(intelPath)) {
        try {
            return JSON.parse(fs.readFileSync(intelPath, 'utf8'));
        } catch (err) {
            console.error('Error loading threat intelligence:', err.message);
            return null;
        }
    }
    return null;
}

/**
 * Analyze specific threat pattern
 */
function analyzePattern(pattern, allPatterns) {
    const relatedPatterns = allPatterns.filter(p =>
        p.rule === pattern.rule && p.cveId !== pattern.cveId
    );

    return {
        pattern,
        relatedCount: relatedPatterns.length,
        avgSeverity: relatedPatterns.reduce((sum, p) => {
            const severityScore = { 'critical': 4, 'high': 3, 'medium': 2, 'low': 1 };
            return sum + (severityScore[p.severity] || 1);
        }, 0) / relatedPatterns.length,
        recommendation: generatePatternRecommendation(pattern, relatedPatterns)
    };
}

/**
 * Generate recommendations for specific patterns
 */
function generatePatternRecommendation(pattern, relatedPatterns) {
    const recommendations = {
        'Code Execution': 'Implement strict input validation and code review processes',
        'Injection': 'Use parameterized queries and input sanitization',
        'Security Bypass': 'Review authentication and authorization mechanisms',
        'Privilege Escalation': 'Implement principle of least privilege',
        'Crypto Weakness': 'Update to modern cryptographic standards',
        'Buffer Overflow': 'Use memory-safe programming languages or tools',
        'Zero-day': 'Implement advanced threat detection and monitoring'
    };

    return recommendations[pattern.rule] || 'Review security configurations and apply patches';
}

module.exports = {
    extractPatternsFromCVEs,
    loadLearnedPatterns,
    getThreatIntelligence,
    analyzePattern
};
