#include "pattern_engine.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

namespace harmony {

// PatternEngine Implementation

PatternEngine::PatternEngine(int k) : numClusters(k) {
    std::cout << "🧠 Pattern Recognition Engine initialized (K=" << k << ")" << std::endl;
}

PatternEngine::~PatternEngine() {
    std::cout << "🧠 Pattern Recognition Engine shutdown" << std::endl;
}

void PatternEngine::addTrainingPattern(const ThreatPattern& pattern) {
    trainingData.push_back(pattern);
}

double PatternEngine::euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) return std::numeric_limits<double>::max();
    
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

std::vector<double> PatternEngine::extractFeatures(const ThreatPattern& pattern) {
    std::vector<double> features;
    
    // Feature 1: Severity
    features.push_back(pattern.severity);
    
    // Feature 2: Keyword count
    features.push_back(static_cast<double>(pattern.keywords.size()));
    
    // Feature 3: Description length
    features.push_back(static_cast<double>(pattern.description.length()) / 100.0);
    
    // Add custom features if provided
    for (double f : pattern.features) {
        features.push_back(f);
    }
    
    return features;
}

int PatternEngine::findNearestCluster(const std::vector<double>& features) {
    int nearest = 0;
    double minDist = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < clusters.size(); i++) {
        double dist = euclideanDistance(features, clusters[i].centroid);
        if (dist < minDist) {
            minDist = dist;
            nearest = i;
        }
    }
    
    return nearest;
}

void PatternEngine::updateCentroids() {
    for (auto& cluster : clusters) {
        if (cluster.patterns.empty()) continue;
        
        size_t featureCount = cluster.centroid.size();
        std::vector<double> newCentroid(featureCount, 0.0);
        
        // Average all pattern features
        for (const auto& pattern : cluster.patterns) {
            auto features = extractFeatures(pattern);
            for (size_t i = 0; i < featureCount && i < features.size(); i++) {
                newCentroid[i] += features[i];
            }
        }
        
        for (size_t i = 0; i < featureCount; i++) {
            newCentroid[i] /= cluster.patterns.size();
        }
        
        cluster.centroid = newCentroid;
    }
}

std::vector<Cluster> PatternEngine::clusterThreats() {
    if (trainingData.empty()) {
        std::cerr << "⚠️ No training data available" << std::endl;
        return clusters;
    }
    
    std::cout << "🔍 Clustering " << trainingData.size() << " threat patterns..." << std::endl;
    
    // Initialize clusters with random centroids
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, trainingData.size() - 1);
    
    clusters.clear();
    for (int i = 0; i < numClusters; i++) {
        auto randomPattern = trainingData[dis(gen)];
        auto features = extractFeatures(randomPattern);
        clusters.emplace_back(i, features);
    }
    
    // K-means iterations
    int maxIterations = 100;
    for (int iter = 0; iter < maxIterations; iter++) {
        // Clear cluster assignments
        for (auto& cluster : clusters) {
            cluster.patterns.clear();
        }
        
        // Assign patterns to nearest cluster
        for (const auto& pattern : trainingData) {
            auto features = extractFeatures(pattern);
            int nearest = findNearestCluster(features);
            clusters[nearest].patterns.push_back(pattern);
        }
        
        // Update centroids
        updateCentroids();
        
        // Check convergence (simplified)
        if (iter % 10 == 0) {
            std::cout << "  Iteration " << iter << ": " << clusters.size() << " clusters" << std::endl;
        }
    }
    
    // Calculate cluster statistics
    for (auto& cluster : clusters) {
        if (cluster.patterns.empty()) continue;
        
        // Find dominant category
        std::map<std::string, int> categoryCount;
        double totalSeverity = 0.0;
        
        for (const auto& pattern : cluster.patterns) {
            categoryCount[pattern.category]++;
            totalSeverity += pattern.severity;
        }
        
        // Find most common category
        int maxCount = 0;
        for (const auto& [cat, count] : categoryCount) {
            if (count > maxCount) {
                maxCount = count;
                cluster.dominantCategory = cat;
            }
        }
        
        cluster.avgSeverity = totalSeverity / cluster.patterns.size();
    }
    
    std::cout << "✅ Clustering complete!" << std::endl;
    return clusters;
}

double PatternEngine::matchPattern(const ThreatPattern& a, const ThreatPattern& b) {
    auto featuresA = extractFeatures(a);
    auto featuresB = extractFeatures(b);
    
    double distance = euclideanDistance(featuresA, featuresB);
    
    // Convert distance to similarity score (0-1)
    double similarity = 1.0 / (1.0 + distance);
    
    // Boost similarity if categories match
    if (a.category == b.category) {
        similarity *= 1.2;
    }
    
    return std::min(1.0, similarity);
}

ThreatPattern PatternEngine::findSimilarPattern(const ThreatPattern& query) {
    double maxSimilarity = 0.0;
    ThreatPattern mostSimilar;
    
    for (const auto& pattern : trainingData) {
        double similarity = matchPattern(query, pattern);
        if (similarity > maxSimilarity) {
            maxSimilarity = similarity;
            mostSimilar = pattern;
        }
    }
    
    std::cout << "🔍 Found similar pattern with " << (maxSimilarity * 100) << "% match" << std::endl;
    return mostSimilar;
}

bool PatternEngine::isAnomaly(const ThreatPattern& pattern, double threshold) {
    auto features = extractFeatures(pattern);
    
    if (clusters.empty()) {
        return false;  // Can't detect anomalies without clusters
    }
    
    int nearest = findNearestCluster(features);
    double distance = euclideanDistance(features, clusters[nearest].centroid);
    
    // Calculate average distance in cluster
    double avgDistance = 0.0;
    for (const auto& p : clusters[nearest].patterns) {
        auto pFeatures = extractFeatures(p);
        avgDistance += euclideanDistance(pFeatures, clusters[nearest].centroid);
    }
    avgDistance /= clusters[nearest].patterns.size();
    
    // Anomaly if distance > threshold * average
    return distance > (threshold * avgDistance);
}

std::string PatternEngine::classifyThreat(const ThreatPattern& pattern) {
    if (clusters.empty()) {
        clusterThreats();
    }
    
    auto features = extractFeatures(pattern);
    int nearest = findNearestCluster(features);
    
    return clusters[nearest].dominantCategory;
}

double PatternEngine::predictSeverity(const ThreatPattern& pattern) {
    if (clusters.empty()) {
        clusterThreats();
    }
    
    auto features = extractFeatures(pattern);
    int nearest = findNearestCluster(features);
    
    return clusters[nearest].avgSeverity;
}

void PatternEngine::printClusterStats() {
    std::cout << "\n📊 Cluster Statistics:" << std::endl;
    std::cout << "======================" << std::endl;
    
    for (const auto& cluster : clusters) {
        if (cluster.patterns.empty()) continue;
        
        std::cout << "\nCluster " << cluster.id << ":" << std::endl;
        std::cout << "  Patterns: " << cluster.patterns.size() << std::endl;
        std::cout << "  Category: " << cluster.dominantCategory << std::endl;
        std::cout << "  Avg Severity: " << cluster.avgSeverity << std::endl;
    }
    std::cout << std::endl;
}

// DecisionTree Implementation

DecisionTree::DecisionTree() {
    std::cout << "🌳 Decision Tree initialized" << std::endl;
}

DecisionTree::~DecisionTree() {
    std::cout << "🌳 Decision Tree shutdown" << std::endl;
}

double DecisionTree::calculateEntropy(const std::vector<ThreatPattern>& patterns) {
    if (patterns.empty()) return 0.0;
    
    std::map<std::string, int> categoryCount;
    for (const auto& p : patterns) {
        categoryCount[p.category]++;
    }
    
    double entropy = 0.0;
    for (const auto& [cat, count] : categoryCount) {
        double prob = static_cast<double>(count) / patterns.size();
        if (prob > 0) {
            entropy -= prob * std::log2(prob);
        }
    }
    
    return entropy;
}

void DecisionTree::train(const std::vector<ThreatPattern>& patterns, int maxDepth) {
    std::cout << "🌳 Training decision tree with " << patterns.size() << " patterns..." << std::endl;
    root = buildTree(patterns, 0);
    std::cout << "✅ Decision tree trained!" << std::endl;
}

std::unique_ptr<DecisionTree::Node> DecisionTree::buildTree(
    const std::vector<ThreatPattern>& patterns, int depth) {
    
    auto node = std::make_unique<Node>();
    
    // Base case: all same category or max depth
    if (patterns.empty() || depth > 10) {
        node->isLeaf = true;
        if (!patterns.empty()) {
            node->classification = patterns[0].category;
        }
        return node;
    }
    
    // Check if all same category
    std::string firstCat = patterns[0].category;
    bool allSame = true;
    for (const auto& p : patterns) {
        if (p.category != firstCat) {
            allSame = false;
            break;
        }
    }
    
    if (allSame) {
        node->isLeaf = true;
        node->classification = firstCat;
        return node;
    }
    
    // Split on severity (simple heuristic)
    node->featureName = "severity";
    node->threshold = 5.0;  // Mid-range severity
    
    std::vector<ThreatPattern> left, right;
    for (const auto& p : patterns) {
        if (p.severity < node->threshold) {
            left.push_back(p);
        } else {
            right.push_back(p);
        }
    }
    
    node->left = buildTree(left, depth + 1);
    node->right = buildTree(right, depth + 1);
    
    return node;
}

std::string DecisionTree::classify(const ThreatPattern& pattern) {
    if (!root) {
        return "Unknown";
    }
    return classifyNode(pattern, root.get());
}

std::string DecisionTree::classifyNode(const ThreatPattern& pattern, Node* node) {
    if (node->isLeaf) {
        return node->classification;
    }
    
    if (pattern.severity < node->threshold) {
        return classifyNode(pattern, node->left.get());
    } else {
        return classifyNode(pattern, node->right.get());
    }
}

double DecisionTree::accuracy(const std::vector<ThreatPattern>& testData) {
    if (testData.empty()) return 0.0;
    
    int correct = 0;
    for (const auto& pattern : testData) {
        std::string predicted = classify(pattern);
        if (predicted == pattern.category) {
            correct++;
        }
    }
    
    return static_cast<double>(correct) / testData.size();
}

} // namespace harmony
