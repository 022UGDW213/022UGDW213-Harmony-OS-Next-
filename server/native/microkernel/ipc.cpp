#include "ipc.h"
#include <iostream>
#include <thread>

namespace harmony {

// MessageQueue Implementation

MessageQueue::MessageQueue(size_t max) : nextMessageId(1), maxSize(max) {}

bool MessageQueue::send(int senderPid, int receiverPid, const std::string& type,
                       const std::string& payload, MessagePriority priority) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (queue.size() >= maxSize) {
        std::cerr << "⚠️ Message queue full for PID " << receiverPid << std::endl;
        return false;
    }
    
    auto msg = std::make_shared<Message>(senderPid, receiverPid, type, payload, priority);
    msg->id = nextMessageId++;
    queue.push(msg);
    
    cv.notify_one();
    return true;
}

std::shared_ptr<Message> MessageQueue::receive(int receiverPid, int timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Wait for message with timeout
    if (timeoutMs < 0) {
        // Wait indefinitely
        cv.wait(lock, [this]() { return !queue.empty(); });
    } else if (timeoutMs > 0) {
        // Wait with timeout
        if (!cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                        [this]() { return !queue.empty(); })) {
            return nullptr; // Timeout
        }
    } else {
        // Non-blocking
        if (queue.empty()) {
            return nullptr;
        }
    }
    
    // Get highest priority message
    auto msg = queue.top();
    queue.pop();
    
    return msg;
}

bool MessageQueue::hasMessages(int receiverPid) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    return !queue.empty();
}

size_t MessageQueue::size() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    return queue.size();
}

void MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    while (!queue.empty()) {
        queue.pop();
    }
}

// IPCManager Implementation

IPCManager::IPCManager() {
    std::cout << "📬 IPC Manager initialized" << std::endl;
}

IPCManager::~IPCManager() {
    std::cout << "📬 IPC Manager shutting down" << std::endl;
    processQueues.clear();
}

bool IPCManager::createQueue(int pid, size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (processQueues.find(pid) != processQueues.end()) {
        return false; // Queue already exists
    }
    
    processQueues[pid] = std::make_shared<MessageQueue>(maxSize);
    std::cout << "📬 Created message queue for PID " << pid << std::endl;
    return true;
}

bool IPCManager::removeQueue(int pid) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = processQueues.find(pid);
    if (it == processQueues.end()) {
        return false;
    }
    
    processQueues.erase(it);
    std::cout << "📬 Removed message queue for PID " << pid << std::endl;
    return true;
}

bool IPCManager::sendMessage(int senderPid, int receiverPid, const std::string& type,
                             const std::string& payload, MessagePriority priority) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = processQueues.find(receiverPid);
    if (it == processQueues.end()) {
        std::cerr << "❌ No queue for receiver PID " << receiverPid << std::endl;
        return false;
    }
    
    bool sent = it->second->send(senderPid, receiverPid, type, payload, priority);
    if (sent) {
        std::cout << "📨 Message sent: " << senderPid << " → " << receiverPid 
                  << " [" << type << "]" << std::endl;
    }
    
    return sent;
}

std::shared_ptr<Message> IPCManager::receiveMessage(int receiverPid, int timeoutMs) {
    std::shared_ptr<MessageQueue> queue;
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = processQueues.find(receiverPid);
        if (it == processQueues.end()) {
            return nullptr;
        }
        queue = it->second;
    }
    
    auto msg = queue->receive(receiverPid, timeoutMs);
    if (msg) {
        std::cout << "📬 Message received by PID " << receiverPid 
                  << " from PID " << msg->senderPid << std::endl;
    }
    
    return msg;
}

void IPCManager::broadcast(int senderPid, const std::string& type, const std::string& payload) {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::cout << "📢 Broadcasting message from PID " << senderPid << std::endl;
    
    for (auto& [pid, queue] : processQueues) {
        if (pid != senderPid) {
            queue->send(senderPid, pid, type, payload, MessagePriority::NORMAL);
        }
    }
}

size_t IPCManager::getTotalMessages() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
    
    size_t total = 0;
    for (const auto& [pid, queue] : processQueues) {
        total += queue->size();
    }
    return total;
}

} // namespace harmony
