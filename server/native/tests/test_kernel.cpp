#include "microkernel/kernel.h"
#include "microkernel/ipc.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace harmony;

void testKernel() {
    std::cout << "\n=== Testing Kernel ===" << std::endl;
    
    Kernel kernel;
    std::cout << kernel.getKernelInfo() << std::endl;
    
    // Create processes
    int pid1 = kernel.createProcess("WebRenderer", Priority::HIGH);
    int pid2 = kernel.createProcess("NetworkService", Priority::NORMAL);
    int pid3 = kernel.createProcess("GPUProcess", Priority::CRITICAL);
    
    std::cout << "\nProcess count: " << kernel.getProcessCount() << std::endl;
    
    // Test scheduling
    std::cout << "\n--- Scheduling Test ---" << std::endl;
    for (int i = 0; i < 5; i++) {
        int scheduled = kernel.schedule();
        auto pcb = kernel.getProcess(scheduled);
        if (pcb) {
            std::cout << "Scheduled PID " << scheduled << " (" << pcb->name 
                      << ") Priority=" << static_cast<int>(pcb->priority) << std::endl;
        }
        kernel.yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Terminate a process
    kernel.terminateProcess(pid2);
    std::cout << "\nProcess count after termination: " << kernel.getProcessCount() << std::endl;
}

void testIPC() {
    std::cout << "\n\n=== Testing IPC ===" << std::endl;
    
    IPCManager ipc;
    
    // Create queues for processes
    ipc.createQueue(1);
    ipc.createQueue(2);
    ipc.createQueue(3);
    
    // Send messages with different priorities
    std::cout << "\n--- Sending Messages ---" << std::endl;
    ipc.sendMessage(1, 2, "REQUEST", "Low priority request", MessagePriority::LOW);
    ipc.sendMessage(1, 2, "ALERT", "Critical alert!", MessagePriority::CRITICAL);
    ipc.sendMessage(1, 2, "DATA", "Normal data", MessagePriority::NORMAL);
    ipc.sendMessage(3, 2, "RENDER", "High priority render", MessagePriority::HIGH);
    
    // Receive messages (should come in priority order)
    std::cout << "\n--- Receiving Messages (Priority Order) ---" << std::endl;
    for (int i = 0; i < 4; i++) {
        auto msg = ipc.receiveMessage(2, 1000);
        if (msg) {
            std::cout << "Received: [" << msg->type << "] from PID " << msg->senderPid
                      << " Priority=" << static_cast<int>(msg->priority)
                      << " Payload=\"" << msg->payload << "\"" << std::endl;
        }
    }
    
    // Test broadcast
    std::cout << "\n--- Testing Broadcast ---" << std::endl;
    ipc.broadcast(1, "SHUTDOWN", "System shutting down");
    
    std::cout << "Total messages in system: " << ipc.getTotalMessages() << std::endl;
}

void testConcurrentIPC() {
    std::cout << "\n\n=== Testing Concurrent IPC ===" << std::endl;
    
    IPCManager ipc;
    ipc.createQueue(100); // Receiver
    ipc.createQueue(101); // Sender 1
    ipc.createQueue(102); // Sender 2
    
    // Sender threads
    auto sender = [&ipc](int senderPid, int count) {
        for (int i = 0; i < count; i++) {
            std::string payload = "Message " + std::to_string(i) + " from PID " + std::to_string(senderPid);
            ipc.sendMessage(senderPid, 100, "DATA", payload);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    
    // Receiver thread
    auto receiver = [&ipc](int receiverPid, int expected) {
        int received = 0;
        while (received < expected) {
            auto msg = ipc.receiveMessage(receiverPid, 2000);
            if (msg) {
                received++;
                std::cout << "Received " << received << "/" << expected << ": " << msg->payload << std::endl;
            } else {
                std::cout << "Timeout waiting for message" << std::endl;
                break;
            }
        }
    };
    
    std::cout << "Starting concurrent send/receive test..." << std::endl;
    
    std::thread t1(sender, 101, 5);
    std::thread t2(sender, 102, 5);
    std::thread t3(receiver, 100, 10);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Concurrent test completed!" << std::endl;
}

int main() {
    std::cout << "🔷 Harmony Microkernel Test Suite 🔷" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        testKernel();
        testIPC();
        testConcurrentIPC();
        
        std::cout << "\n\n✅ All tests completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
