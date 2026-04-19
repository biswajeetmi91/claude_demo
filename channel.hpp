#pragma once
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <string>
#include "message.hpp"

class Channel {
public:
    void subscribe(const std::string& agentId) {
        std::lock_guard<std::mutex> lock(mutex_);
        queues_[agentId];
    }

    void publish(const Message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (msg.to.empty()) {
            for (auto& [id, q] : queues_)
                if (id != msg.from) q.push(msg);
        } else {
            auto it = queues_.find(msg.to);
            if (it != queues_.end()) it->second.push(msg);
        }
        cv_.notify_all();
    }

    // Blocks until a message is available for agentId
    Message receive(const std::string& agentId) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return !queues_[agentId].empty(); });
        Message msg = queues_[agentId].front();
        queues_[agentId].pop();
        return msg;
    }

    bool tryReceive(const std::string& agentId, Message& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& q = queues_[agentId];
        if (q.empty()) return false;
        out = q.front();
        q.pop();
        return true;
    }

private:
    std::unordered_map<std::string, std::queue<Message>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
