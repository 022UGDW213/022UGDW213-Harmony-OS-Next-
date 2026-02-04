#include "process.h"
#include <iostream>
#include <chrono>

namespace harmony {

// Base Process Implementation

Process::Process(const std::string& n, ProcessType t, Priority p, 
                Kernel* k, IPCManager* ipc)
    : pid(0), name(n), type(t), priority(p), kernel(k), 
      ipcManager(ipc), running(false) {
}

Process::~Process() {
    if (running) {
        stop();
    }
}

void Process::start() {
    if (running) return;
    
    // Create process in kernel
    pid = kernel->createProcess(name, priority);
    
    // Create IPC queue
    ipcManager->createQueue(pid);
    
    // Start worker thread
    running = true;
    workerThread = std::thread(&Process::processLoop, this);
    
    std::cout << "🚀 Started " << name << " (PID " << pid << ")" << std::endl;
}

void Process::stop() {
    if (!running) return;
    
    running = false;
    
    // Send shutdown message to wake up thread
    sendMessage(pid, "SHUTDOWN", "", MessagePriority::CRITICAL);
    
    if (workerThread.joinable()) {
        workerThread.join();
    }
    
    // Cleanup
    ipcManager->removeQueue(pid);
    kernel->terminateProcess(pid);
    
    std::cout << "🛑 Stopped " << name << " (PID " << pid << ")" << std::endl;
}

void Process::processLoop() {
    while (running) {
        // Receive messages with timeout
        auto msg = ipcManager->receiveMessage(pid, 100);
        
        if (msg) {
            if (msg->type == "SHUTDOWN") {
                break;
            }
            handleMessage(msg);
        }
        
        // Yield to scheduler
        kernel->yield();
    }
}

bool Process::sendMessage(int receiverPid, const std::string& type,
                         const std::string& payload, MessagePriority msgPriority) {
    return ipcManager->sendMessage(pid, receiverPid, type, payload, msgPriority);
}

// Renderer Process Implementation

RendererProcess::RendererProcess(Kernel* k, IPCManager* ipc)
    : Process("RendererProcess", ProcessType::RENDERER, Priority::HIGH, k, ipc) {
}

void RendererProcess::handleMessage(std::shared_ptr<Message> msg) {
    if (msg->type == "RENDER") {
        renderFrame(msg->payload);
    } else if (msg->type == "GPU_RESULT") {
        std::cout << "  🎨 Renderer received GPU result: " << msg->payload << std::endl;
    }
}

void RendererProcess::renderFrame(const std::string& data) {
    std::cout << "  🖼️  Rendering frame: " << data << std::endl;
    
    // Simulate rendering work
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "  ✅ Frame rendered successfully" << std::endl;
}

void RendererProcess::requestRender(const std::string& html) {
    sendMessage(pid, "RENDER", html, MessagePriority::HIGH);
}

// GPU Process Implementation

GPUProcess::GPUProcess(Kernel* k, IPCManager* ipc)
    : Process("GPUProcess", ProcessType::GPU, Priority::CRITICAL, k, ipc) {
}

void GPUProcess::handleMessage(std::shared_ptr<Message> msg) {
    if (msg->type == "ACCELERATE") {
        processGraphics(msg->payload);
        // Send result back to sender
        sendMessage(msg->senderPid, "GPU_RESULT", "Accelerated: " + msg->payload, 
                   MessagePriority::HIGH);
    }
}

void GPUProcess::processGraphics(const std::string& commands) {
    std::cout << "  🎮 GPU processing: " << commands << std::endl;
    
    // Simulate GPU work
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    
    std::cout << "  ⚡ GPU acceleration complete" << std::endl;
}

void GPUProcess::accelerate(const std::string& operation) {
    sendMessage(pid, "ACCELERATE", operation, MessagePriority::CRITICAL);
}

// Network Process Implementation

NetworkProcess::NetworkProcess(Kernel* k, IPCManager* ipc)
    : Process("NetworkProcess", ProcessType::NETWORK, Priority::NORMAL, k, ipc) {
}

void NetworkProcess::handleMessage(std::shared_ptr<Message> msg) {
    if (msg->type == "FETCH") {
        handleRequest(msg->payload);
        // Send result back
        sendMessage(msg->senderPid, "FETCH_RESULT", 
                   "Data from: " + msg->payload, MessagePriority::NORMAL);
    }
}

void NetworkProcess::handleRequest(const std::string& url) {
    std::cout << "  🌐 Fetching: " << url << std::endl;
    
    // Simulate network request
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "  📦 Fetch complete" << std::endl;
}

void NetworkProcess::fetch(const std::string& url) {
    sendMessage(pid, "FETCH", url, MessagePriority::NORMAL);
}

// Process Manager Implementation

ProcessManager::ProcessManager() {
    std::cout << "🏗️  Process Manager initialized" << std::endl;
}

ProcessManager::~ProcessManager() {
    stopAll();
    std::cout << "🏗️  Process Manager shutdown" << std::endl;
}

RendererProcess* ProcessManager::createRenderer() {
    auto renderer = std::make_unique<RendererProcess>(&kernel, &ipcManager);
    auto* ptr = renderer.get();
    processes.push_back(std::move(renderer));
    return ptr;
}

GPUProcess* ProcessManager::createGPU() {
    auto gpu = std::make_unique<GPUProcess>(&kernel, &ipcManager);
    auto* ptr = gpu.get();
    processes.push_back(std::move(gpu));
    return ptr;
}

NetworkProcess* ProcessManager::createNetwork() {
    auto network = std::make_unique<NetworkProcess>(&kernel, &ipcManager);
    auto* ptr = network.get();
    processes.push_back(std::move(network));
    return ptr;
}

void ProcessManager::startAll() {
    std::cout << "\n🚀 Starting all processes..." << std::endl;
    for (auto& process : processes) {
        process->start();
    }
    std::cout << "✅ All processes started\n" << std::endl;
}

void ProcessManager::stopAll() {
    std::cout << "\n🛑 Stopping all processes..." << std::endl;
    for (auto& process : processes) {
        process->stop();
    }
    processes.clear();
    std::cout << "✅ All processes stopped\n" << std::endl;
}

size_t ProcessManager::getProcessCount() const {
    return processes.size();
}

void ProcessManager::printStatus() const {
    std::cout << "\n📊 Process Status:" << std::endl;
    std::cout << "Total processes: " << processes.size() << std::endl;
    for (const auto& process : processes) {
        std::cout << "  - " << process->getName() 
                  << " (PID " << process->getPid() << ") "
                  << (process->isRunning() ? "✅ Running" : "⏸️  Stopped")
                  << std::endl;
    }
    std::cout << std::endl;
}

} // namespace harmony
