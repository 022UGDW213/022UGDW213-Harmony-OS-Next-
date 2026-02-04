#ifndef HARMONY_PROCESS_H
#define HARMONY_PROCESS_H

#include "../microkernel/kernel.h"
#include "../microkernel/ipc.h"
#include <string>
#include <functional>
#include <thread>
#include <atomic>

namespace harmony {

// Process types
enum class ProcessType {
    RENDERER,
    GPU,
    NETWORK,
    BROWSER,
    UTILITY
};

// Base Process class
class Process {
protected:
    int pid;
    std::string name;
    ProcessType type;
    Priority priority;
    Kernel* kernel;
    IPCManager* ipcManager;
    std::atomic<bool> running;
    std::thread workerThread;
    
    // Message handler
    virtual void handleMessage(std::shared_ptr<Message> msg) = 0;
    
    // Main process loop
    void processLoop();
    
public:
    Process(const std::string& n, ProcessType t, Priority p, 
            Kernel* k, IPCManager* ipc);
    virtual ~Process();
    
    // Lifecycle
    virtual void start();
    virtual void stop();
    
    // Communication
    bool sendMessage(int receiverPid, const std::string& type, 
                    const std::string& payload, 
                    MessagePriority msgPriority = MessagePriority::NORMAL);
    
    // Getters
    int getPid() const { return pid; }
    std::string getName() const { return name; }
    ProcessType getType() const { return type; }
    bool isRunning() const { return running; }
};

// Renderer Process (handles UI rendering)
class RendererProcess : public Process {
private:
    void handleMessage(std::shared_ptr<Message> msg) override;
    void renderFrame(const std::string& data);
    
public:
    RendererProcess(Kernel* k, IPCManager* ipc);
    void requestRender(const std::string& html);
};

// GPU Process (handles graphics acceleration)
class GPUProcess : public Process {
private:
    void handleMessage(std::shared_ptr<Message> msg) override;
    void processGraphics(const std::string& commands);
    
public:
    GPUProcess(Kernel* k, IPCManager* ipc);
    void accelerate(const std::string& operation);
};

// Network Process (handles network requests)
class NetworkProcess : public Process {
private:
    void handleMessage(std::shared_ptr<Message> msg) override;
    void handleRequest(const std::string& url);
    
public:
    NetworkProcess(Kernel* k, IPCManager* ipc);
    void fetch(const std::string& url);
};

// Process Manager
class ProcessManager {
private:
    Kernel kernel;
    IPCManager ipcManager;
    std::vector<std::unique_ptr<Process>> processes;
    
public:
    ProcessManager();
    ~ProcessManager();
    
    // Create processes
    RendererProcess* createRenderer();
    GPUProcess* createGPU();
    NetworkProcess* createNetwork();
    
    // Lifecycle
    void startAll();
    void stopAll();
    
    // Info
    size_t getProcessCount() const;
    void printStatus() const;
};

} // namespace harmony

#endif // HARMONY_PROCESS_H
