#include "learning/pattern_engine.h"
#include <iostream>
#include <vector>

using namespace harmony;

void demonstratePatternRecognition() {
    std::cout << "🧠 Pattern Recognition Engine Demo 🧠" << std::endl;
    std::cout << "====================================\n" << std::endl;
    
    // Create pattern engine
    PatternEngine engine(3);  // 3 clusters
    
    // Create sample threat patterns (simulating CVE data)
    std::cout << "📦 Creating sample threat patterns..." << std::endl;
    
    // Cluster 1: Remote Code Execution (High severity)
    engine.addTrainingPattern({
        "CVE-2024-0001", 
        "Remote code execution via buffer overflow",
        {"buffer", "overflow", "remote", "execution"},
        {},
        9.8,
        "Code Execution"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0002",
        "RCE through deserialization",
        {"deserialization", "remote", "code"},
        {},
        9.5,
        "Code Execution"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0003",
        "Command injection vulnerability",
        {"command", "injection", "execution"},
        {},
        9.0,
        "Code Execution"
    });
    
    // Cluster 2: SQL Injection (Medium severity)
    engine.addTrainingPattern({
        "CVE-2024-0004",
        "SQL injection in login form",
        {"sql", "injection", "database"},
        {},
        7.5,
        "Injection"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0005",
        "Blind SQL injection",
        {"sql", "blind", "injection"},
        {},
        7.0,
        "Injection"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0006",
        "NoSQL injection vulnerability",
        {"nosql", "injection", "mongodb"},
        {},
        6.8,
        "Injection"
    });
    
    // Cluster 3: XSS (Lower severity)
    engine.addTrainingPattern({
        "CVE-2024-0007",
        "Cross-site scripting in comments",
        {"xss", "cross-site", "scripting"},
        {},
        5.5,
        "XSS"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0008",
        "Stored XSS vulnerability",
        {"stored", "xss", "persistent"},
        {},
        6.0,
        "XSS"
    });
    
    engine.addTrainingPattern({
        "CVE-2024-0009",
        "Reflected XSS attack",
        {"reflected", "xss"},
        {},
        5.0,
        "XSS"
    });
    
    std::cout << "✅ Created 9 threat patterns\n" << std::endl;
    
    // Perform clustering
    std::cout << "🔬 Scenario 1: K-Means Clustering" << std::endl;
    std::cout << "===================================" << std::endl;
    auto clusters = engine.clusterThreats();
    engine.printClusterStats();
    
    // Test pattern matching
    std::cout << "🔬 Scenario 2: Pattern Matching" << std::endl;
    std::cout << "================================" << std::endl;
    
    ThreatPattern newThreat{
        "CVE-2024-NEW",
        "Buffer overflow in network service",
        {"buffer", "overflow", "network"},
        {},
        9.2,
        "Unknown"
    };
    
    std::cout << "Query pattern: " << newThreat.description << std::endl;
    auto similar = engine.findSimilarPattern(newThreat);
    std::cout << "Most similar: " << similar.id << " - " << similar.description << std::endl;
    std::cout << std::endl;
    
    // Test classification
    std::cout << "🔬 Scenario 3: Threat Classification" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    std::string predicted = engine.classifyThreat(newThreat);
    std::cout << "Predicted category: " << predicted << std::endl;
    
    double predictedSeverity = engine.predictSeverity(newThreat);
    std::cout << "Predicted severity: " << predictedSeverity << std::endl;
    std::cout << "Actual severity: " << newThreat.severity << std::endl;
    std::cout << std::endl;
    
    // Test anomaly detection
    std::cout << "🔬 Scenario 4: Anomaly Detection" << std::endl;
    std::cout << "=================================" << std::endl;
    
    ThreatPattern normalPattern{
        "CVE-2024-NORMAL",
        "Standard SQL injection",
        {"sql", "injection"},
        {},
        7.0,
        "Injection"
    };
    
    ThreatPattern anomalyPattern{
        "CVE-2024-ANOMALY",
        "Extremely rare quantum cryptography attack",
        {"quantum", "cryptography", "attack"},
        {},
        10.0,
        "Quantum"
    };
    
    bool isNormalAnomaly = engine.isAnomaly(normalPattern);
    bool isAnomalyAnomaly = engine.isAnomaly(anomalyPattern);
    
    std::cout << "Normal pattern anomaly: " << (isNormalAnomaly ? "YES ⚠️" : "NO ✅") << std::endl;
    std::cout << "Unusual pattern anomaly: " << (isAnomalyAnomaly ? "YES ⚠️" : "NO ✅") << std::endl;
    std::cout << std::endl;
    
    // Decision Tree Demo
    std::cout << "🔬 Scenario 5: Decision Tree Classification" << std::endl;
    std::cout << "============================================" << std::endl;
    
    DecisionTree tree;
    
    // Create training data
    std::vector<ThreatPattern> trainingData;
    for (int i = 0; i < 9; i++) {
        ThreatPattern p;
        p.id = "TRAIN-" + std::to_string(i);
        
        if (i < 3) {
            p.severity = 9.0 + (i * 0.2);
            p.category = "Code Execution";
        } else if (i < 6) {
            p.severity = 7.0 + (i * 0.1);
            p.category = "Injection";
        } else {
            p.severity = 5.0 + (i * 0.2);
            p.category = "XSS";
        }
        
        trainingData.push_back(p);
    }
    
    tree.train(trainingData);
    
    // Test classification
    ThreatPattern testPattern{
        "TEST-001",
        "Test vulnerability",
        {},
        {},
        8.5,
        "Code Execution"
    };
    
    std::string treeClassification = tree.classify(testPattern);
    std::cout << "Decision tree classification: " << treeClassification << std::endl;
    std::cout << "Actual category: " << testPattern.category << std::endl;
    
    double acc = tree.accuracy(trainingData);
    std::cout << "Training accuracy: " << (acc * 100) << "%" << std::endl;
    std::cout << std::endl;
    
    std::cout << "✅ Pattern Recognition Demo Complete!" << std::endl;
}

int main() {
    try {
        demonstratePatternRecognition();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
