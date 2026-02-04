#ifndef HARMONY_IPC_H
#define HARMONY_IPC_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

namespace harmony {

// Message priority
enum class MessagePriority {
    CRITICAL = 0,
    HIGH = 1,
    NORMAL = 2,
    LOW = 3
};

// IPC Message structure
struct Message {
    int id;
    int senderPid;
    int receiverPid;
    MessagePriority priority;
    std::string type;
    std::string payload;
    std::chrono::steady_clock::time_point timestamp;
    
    Message(int sid, int rid, const std::string& t, const std::string& p, 
            MessagePriority pri = MessagePriority::NORMAL)
        : id(0), senderPid(sid), receiverPid(rid), priority(pri),
          type(t), payload(p), timestamp(std::chrono::steady_clock::now()) {}
};

// Message comparison for priority queue
struct MessageComparator {
    bool operator()(const std::shared_ptr<Message>& a, 
                   const std::shared_ptr<Message>& b) const {
        if (a->priority != b->priority) {
            return a->priority > b->priority; // Lower priority value = higher priority
        }
        return a->timestamp > b->timestamp; // FIFO for same priority
    }
};

// IPC Message Queue
class MessageQueue {
private:
    std::priority_queue<std::shared_ptr<Message>, 
                       std::vector<std::shared_ptr<Message>>,
                       MessageComparator> queue;
    std::mutex mutex;
    std::condition_variable cv;
    int nextMessageId;
    size_t maxSize;
    
public:
    MessageQueue(size_t max = 1000);
    
    // Send message (non-blocking)
    bool send(int senderPid, int receiverPid, const std::string& type,
              const std::string& payload, MessagePriority priority = MessagePriority::NORMAL);
    
    // Receive message (blocking with timeout)
    std::shared_ptr<Message> receive(int receiverPid, int timeoutMs = -1);
    
    // Check if messages available for a process
    bool hasMessages(int receiverPid) const;
    
    // Get queue size
    size_t size() const;
    
    // Clear all messages
    void clear();
};

// IPC Manager
class IPCManager {
private:
    std::map<int, std::shared_ptr<MessageQueue>> processQueues;
    std::mutex mutex;
    
public:
    IPCManager();
    ~IPCManager();
    
    // Create queue for a process
    bool createQueue(int pid, size_t maxSize = 1000);
    
    // Remove queue for a process
    bool removeQueue(int pid);
    
    // Send message between processes
    bool sendMessage(int senderPid, int receiverPid, const std::string& type,
                    const std::string& payload, MessagePriority priority = MessagePriority::NORMAL);
    
    // Receive message for a process
    std::shared_ptr<Message> receiveMessage(int receiverPid, int timeoutMs = -1);
    
    // Broadcast message to all processes
    void broadcast(int senderPid, const std::string& type, const std::string& payload);
    
    // Get statistics
    size_t getTotalMessages() const;
};

} // namespace harmony

#endif // HARMONY_IPC_H
