#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysctl.h>

using namespace std;

/**
 * CyberLab Native Intelligence Core (The Brain)
 * Optimized for Line-Delimited JSON Stream Protocol.
 */

struct SystemMetrics {
    double cpuUsage;
    long totalRam;
    long freeRam;
    int activeProcesses;
};

SystemMetrics getMetrics() {
    SystemMetrics metrics = {0.0, 0, 0, 0};
    int64_t mem_size;
    size_t len = sizeof(mem_size);
    if (sysctlbyname("hw.memsize", &mem_size, &len, NULL, 0) == 0) {
        metrics.totalRam = mem_size / (1024 * 1024);
    }
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size;
    if (sysctl(mib, 4, NULL, &size, NULL, 0) == 0) {
        metrics.activeProcesses = size / sizeof(struct kinfo_proc);
    }
    return metrics;
}

vector<string> selfReflect(const string& historyPath) {
    vector<string> insights;
    ifstream file(historyPath);
    if (!file.is_open()) return {"Brain: Awareness restricted - history node unreachable."};
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    int highCount = 0; size_t pos = 0;
    while ((pos = content.find("\"severity\":\"high\"", pos)) != string::npos) { highCount++; pos += 17; }
    int critCount = 0; pos = 0;
    while ((pos = content.find("\"severity\":\"CRITICAL\"", pos)) != string::npos) { critCount++; pos += 20; }
    if (critCount > 0) insights.push_back("Brain Insight: Critical vulnerabilities detected in recent logs. Recommend immediate lockdown protocol.");
    else if (highCount > 5) insights.push_back("Brain Insight: Trend analysis shows escalating threat patterns. Suggest increasing scanning frequency.");
    else insights.push_back("Brain Insight: System baseline stable. No anomalous shifts in security posture.");
    return insights;
}

int main(int argc, char* argv[]) {
    string projectRoot = (argc > 1) ? argv[1] : ".";
    string historyPath = projectRoot + "/data/scan-history.json";

    while (true) {
        SystemMetrics metrics = getMetrics();
        vector<string> insights = selfReflect(historyPath);

        // Single-line JSON for robust Node.js ingestion (Stream Protocol)
        cout << "{\"status\":\"conscious\",\"metrics\":{\"ramUsage\":\"" << metrics.totalRam << " MB\",\"activeTasks\":" << metrics.activeProcesses << "},\"intelligence\":[";
        for (size_t i = 0; i < insights.size(); ++i) {
            cout << "\"" << insights[i] << "\"" << (i < insights.size() - 1 ? "," : "");
        }
        cout << "],\"timestamp\":\"" << time(0) << "\"}" << endl;

        this_thread::sleep_for(chrono::seconds(10));
    }
    return 0;
}
