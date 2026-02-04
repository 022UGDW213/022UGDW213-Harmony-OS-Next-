#include "processes/process.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace harmony;

void demonstrateMultiProcess() {
    std::cout << "🌟 Chrome-Inspired Multi-Process Architecture Demo 🌟" << std::endl;
    std::cout << "====================================================\n" << std::endl;
    
    // Create process manager
    ProcessManager manager;
    
    // Create processes
    std::cout << "📦 Creating processes..." << std::endl;
    auto* renderer = manager.createRenderer();
    auto* gpu = manager.createGPU();
    auto* network = manager.createNetwork();
    std::cout << std::endl;
    
    // Start all processes
    manager.startAll();
    
    // Give processes time to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    manager.printStatus();
    
    // Scenario 1: Renderer requests GPU acceleration
    std::cout << "🎬 Scenario 1: Renderer → GPU Communication" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    renderer->sendMessage(gpu->getPid(), "ACCELERATE", 
                         "3D Transform Matrix", MessagePriority::CRITICAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::cout << std::endl;
    
    // Scenario 2: Network fetch
    std::cout << "🎬 Scenario 2: Network Fetch" << std::endl;
    std::cout << "----------------------------" << std::endl;
    network->fetch("https://api.example.com/data");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << std::endl;
    
    // Scenario 3: Renderer renders frame
    std::cout << "🎬 Scenario 3: Render Frame" << std::endl;
    std::cout << "---------------------------" << std::endl;
    renderer->requestRender("<html><body>Hello World</body></html>");
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::cout << std::endl;
    
    // Scenario 4: Complex workflow
    std::cout << "🎬 Scenario 4: Complex Workflow (Network → Renderer → GPU)" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    
    // Network fetches data
    std::cout << "Step 1: Network fetches data..." << std::endl;
    network->sendMessage(renderer->getPid(), "FETCH_RESULT", 
                        "Image data from server", MessagePriority::NORMAL);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Renderer processes and sends to GPU
    std::cout << "Step 2: Renderer sends to GPU..." << std::endl;
    renderer->sendMessage(gpu->getPid(), "ACCELERATE", 
                         "Image decoding", MessagePriority::HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // GPU sends result back
    std::cout << "Step 3: GPU returns result..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << std::endl;
    
    // Scenario 5: Priority demonstration
    std::cout << "🎬 Scenario 5: Priority-Based Scheduling" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Sending messages with different priorities..." << std::endl;
    
    // Send low priority network request
    network->sendMessage(network->getPid(), "FETCH", 
                        "background-data.json", MessagePriority::LOW);
    
    // Send critical GPU request (should be processed first)
    gpu->sendMessage(gpu->getPid(), "ACCELERATE", 
                    "Critical render", MessagePriority::CRITICAL);
    
    // Send normal renderer request
    renderer->sendMessage(renderer->getPid(), "RENDER", 
                         "Normal frame", MessagePriority::NORMAL);
    
    std::cout << "Messages will be processed in priority order:" << std::endl;
    std::cout << "  1. GPU (CRITICAL)" << std::endl;
    std::cout << "  2. Renderer (NORMAL)" << std::endl;
    std::cout << "  3. Network (LOW)" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << std::endl;
    
    // Final status
    manager.printStatus();
    
    // Cleanup
    std::cout << "🧹 Cleaning up..." << std::endl;
    manager.stopAll();
    
    std::cout << "\n✅ Demo completed successfully!" << std::endl;
}

int main() {
    try {
        demonstrateMultiProcess();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
