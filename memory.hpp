#pragma once
#include <unordered_map>
#include <mutex>
#include "interfaces.hpp"

class MemoryManager : public IMemoryStore {
public:
    void set(const std::string& agentId, const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        store_[agentId][key] = value;
    }

    std::optional<std::string> get(const std::string& agentId, const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto agentIt = store_.find(agentId);
        if (agentIt == store_.end()) return std::nullopt;
        auto keyIt = agentIt->second.find(key);
        if (keyIt == agentIt->second.end()) return std::nullopt;
        return keyIt->second;
    }

    void clear(const std::string& agentId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        store_.erase(agentId);
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> store_;
    std::mutex mutex_;
};
