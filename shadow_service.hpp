#pragma once
#include <unordered_map>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include "agent.hpp"
#include "channel.hpp"
#include "memory.hpp"

class ShadowService {
public:
    ShadowService() = default;
    ~ShadowService() { stopAll(); }

    void registerAgent(std::unique_ptr<Agent> agent) {
        const std::string id = agent->id;
        channel_.subscribe(id);
        agents_[id] = std::move(agent);
    }

    void startAll() {
        for (auto& kv : agents_) {
            const std::string id = kv.first;
            running_[id] = true;
            kv.second->onStart();
            threads_[id] = std::thread([this, id]() { run(id); });
        }
    }

    void stopAll() {
        for (auto& [id, flag] : running_) {
            flag = false;
            // Unblock receive() with a poison-pill shutdown message
            channel_.publish({"system", id, "__shutdown__", ""});
        }
        for (auto& [id, t] : threads_)
            if (t.joinable()) t.join();
        threads_.clear();
        running_.clear();
    }

    Channel& channel() { return channel_; }
    MemoryManager& memory() { return memory_; }

private:
    void run(const std::string& id) {
        auto& agent = *agents_.at(id);
        while (running_[id]) {
            Message msg = channel_.receive(id);
            if (msg.topic == "__shutdown__") break;
            try {
                agent.handleMessage(msg);
            } catch (const std::exception& e) {
                std::cerr << "[ShadowService] Agent " << id << " error: " << e.what() << "\n";
            }
        }
    }

    Channel channel_;
    MemoryManager memory_;
    std::unordered_map<std::string, std::unique_ptr<Agent>> agents_;
    std::unordered_map<std::string, std::thread> threads_;
    std::unordered_map<std::string, std::atomic<bool>> running_;
};
