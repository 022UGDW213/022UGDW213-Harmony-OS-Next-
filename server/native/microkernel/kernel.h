#ifndef HARMONY_KERNEL_H
#define HARMONY_KERNEL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>

namespace harmony {

// Process states
enum class ProcessState {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

// Process priority levels
enum class Priority {
    CRITICAL = 0,
    HIGH = 1,
    NORMAL = 2,
    LOW = 3
};

// Process Control Block
struct ProcessControlBlock {
    int pid;
    std::string name;
    ProcessState state;
    Priority priority;
    std::chrono::steady_clock::time_point createdAt;
    std::chrono::steady_clock::time_point lastScheduled;
    size_t memoryUsage;
    void* context;
    
    ProcessControlBlock(int id, const std::string& n, Priority p = Priority::NORMAL)
        : pid(id), name(n), state(ProcessState::READY), priority(p),
          createdAt(std::chrono::steady_clock::now()),
          lastScheduled(std::chrono::steady_clock::now()),
          memoryUsage(0), context(nullptr) {}
};

// Microkernel Core
class Kernel {
private:
    std::map<int, std::shared_ptr<ProcessControlBlock>> processes;
    int nextPid;
    int currentPid;
    
public:
    Kernel();
    ~Kernel();
    
    // Process management
    int createProcess(const std::string& name, Priority priority = Priority::NORMAL);
    bool terminateProcess(int pid);
    bool setProcessState(int pid, ProcessState state);
    std::shared_ptr<ProcessControlBlock> getProcess(int pid);
    
    // Scheduling
    int schedule();  // Returns next PID to run
    void yield();
    
    // Statistics
    size_t getProcessCount() const;
    std::vector<int> getAllProcessIds() const;
    
    // Kernel info
    std::string getKernelInfo() const;
};

} // namespace harmony

#endif // HARMONY_KERNEL_H
