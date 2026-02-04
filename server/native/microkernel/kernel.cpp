#include "kernel.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace harmony {

Kernel::Kernel() : nextPid(1), currentPid(0) {
    std::cout << "🔷 Harmony Microkernel initialized" << std::endl;
}

Kernel::~Kernel() {
    std::cout << "🔷 Harmony Microkernel shutting down" << std::endl;
    processes.clear();
}

int Kernel::createProcess(const std::string& name, Priority priority) {
    int pid = nextPid++;
    auto pcb = std::make_shared<ProcessControlBlock>(pid, name, priority);
    processes[pid] = pcb;
    
    std::cout << "✅ Process created: PID=" << pid << " Name=" << name 
              << " Priority=" << static_cast<int>(priority) << std::endl;
    
    return pid;
}

bool Kernel::terminateProcess(int pid) {
    auto it = processes.find(pid);
    if (it == processes.end()) {
        return false;
    }
    
    it->second->state = ProcessState::TERMINATED;
    std::cout << "🛑 Process terminated: PID=" << pid << std::endl;
    
    processes.erase(it);
    return true;
}

bool Kernel::setProcessState(int pid, ProcessState state) {
    auto it = processes.find(pid);
    if (it == processes.end()) {
        return false;
    }
    
    it->second->state = state;
    return true;
}

std::shared_ptr<ProcessControlBlock> Kernel::getProcess(int pid) {
    auto it = processes.find(pid);
    if (it == processes.end()) {
        return nullptr;
    }
    return it->second;
}

int Kernel::schedule() {
    // Priority-based scheduling with round-robin for same priority
    std::vector<std::shared_ptr<ProcessControlBlock>> readyProcesses;
    
    for (auto& [pid, pcb] : processes) {
        if (pcb->state == ProcessState::READY) {
            readyProcesses.push_back(pcb);
        }
    }
    
    if (readyProcesses.empty()) {
        return 0; // No process to schedule
    }
    
    // Sort by priority (lower number = higher priority)
    std::sort(readyProcesses.begin(), readyProcesses.end(),
        [](const auto& a, const auto& b) {
            if (a->priority != b->priority) {
                return a->priority < b->priority;
            }
            // Same priority: use round-robin (oldest scheduled first)
            return a->lastScheduled < b->lastScheduled;
        });
    
    auto selected = readyProcesses[0];
    selected->state = ProcessState::RUNNING;
    selected->lastScheduled = std::chrono::steady_clock::now();
    currentPid = selected->pid;
    
    return selected->pid;
}

void Kernel::yield() {
    if (currentPid > 0) {
        auto pcb = getProcess(currentPid);
        if (pcb && pcb->state == ProcessState::RUNNING) {
            pcb->state = ProcessState::READY;
        }
    }
}

size_t Kernel::getProcessCount() const {
    return processes.size();
}

std::vector<int> Kernel::getAllProcessIds() const {
    std::vector<int> pids;
    for (const auto& [pid, pcb] : processes) {
        pids.push_back(pid);
    }
    return pids;
}

std::string Kernel::getKernelInfo() const {
    std::ostringstream oss;
    oss << "Harmony Microkernel v1.0\n";
    oss << "Total Processes: " << processes.size() << "\n";
    oss << "Next PID: " << nextPid << "\n";
    oss << "Current PID: " << currentPid << "\n";
    return oss.str();
}

} // namespace harmony
