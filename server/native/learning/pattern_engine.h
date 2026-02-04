#ifndef HARMONY_PATTERN_ENGINE_H
#define HARMONY_PATTERN_ENGINE_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cmath>

namespace harmony {

// Threat pattern structure
struct ThreatPattern {
    std::string id;
    std::string description;
    std::vector<std::string> keywords;
    std::vector<double> features;  // Numerical feature vector
    double severity;  // 0.0 - 10.0
    std::string category;
    
    ThreatPattern() : severity(0.0) {}
    
    ThreatPattern(const std::string& _id, const std::string& _desc,
                 const std::vector<std::string>& _keywords,
                 const std::vector<double>& _features,
                 double _severity, const std::string& _category)
        : id(_id), description(_desc), keywords(_keywords),
          features(_features), severity(_severity), category(_category) {}
};

// Cluster for K-means
struct Cluster {
    int id;
    std::vector<double> centroid;
    std::vector<ThreatPattern> patterns;
    std::string dominantCategory;
    double avgSeverity;
    
    Cluster(int clusterId, const std::vector<double>& center) 
        : id(clusterId), centroid(center), avgSeverity(0.0) {}
};

// Pattern Recognition Engine
class PatternEngine {
private:
    std::vector<ThreatPattern> trainingData;
    std::vector<Cluster> clusters;
    int numClusters;
    
    // Helper functions
    double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b);
    int findNearestCluster(const std::vector<double>& features);
    void updateCentroids();
    std::vector<double> extractFeatures(const ThreatPattern& pattern);
    
public:
    PatternEngine(int k = 5);
    ~PatternEngine();
    
    // Training
    void addTrainingPattern(const ThreatPattern& pattern);
    void train(int maxIterations = 100);
    
    // K-means clustering
    std::vector<Cluster> clusterThreats();
    
    // Pattern matching
    double matchPattern(const ThreatPattern& a, const ThreatPattern& b);
    ThreatPattern findSimilarPattern(const ThreatPattern& query);
    
    // Anomaly detection
    bool isAnomaly(const ThreatPattern& pattern, double threshold = 2.0);
    
    // Classification
    std::string classifyThreat(const ThreatPattern& pattern);
    double predictSeverity(const ThreatPattern& pattern);
    
    // Statistics
    void printClusterStats();
    int getClusterCount() const { return clusters.size(); }
};

// Simple Decision Tree for classification
class DecisionTree {
private:
    struct Node {
        bool isLeaf;
        std::string featureName;
        double threshold;
        std::string classification;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        
        Node() : isLeaf(false), threshold(0.0) {}
    };
    
    std::unique_ptr<Node> root;
    
    // Helper functions
    double calculateEntropy(const std::vector<ThreatPattern>& patterns);
    double calculateInformationGain(const std::vector<ThreatPattern>& patterns,
                                   const std::string& feature, double threshold);
    std::unique_ptr<Node> buildTree(const std::vector<ThreatPattern>& patterns, int depth);
    std::string classifyNode(const ThreatPattern& pattern, Node* node);
    
public:
    DecisionTree();
    ~DecisionTree();
    
    // Training
    void train(const std::vector<ThreatPattern>& patterns, int maxDepth = 10);
    
    // Prediction
    std::string classify(const ThreatPattern& pattern);
    
    // Evaluation
    double accuracy(const std::vector<ThreatPattern>& testData);
};

} // namespace harmony

#endif // HARMONY_PATTERN_ENGINE_H
