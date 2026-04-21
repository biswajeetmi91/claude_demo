#pragma once
#include <unordered_map>
#include <memory>
#include "agent.hpp"
#include "agent_runner.hpp"
#include "channel.hpp"
#include "memory.hpp"
#include "rdf_store.hpp"

// ShadowService: single responsibility — agent lifecycle orchestration.
// Thread execution is delegated to AgentRunner (SRP).
// Owns concrete implementations of IChannel, IMemoryStore, IKnowledgeStore
// and injects them into agents (DIP — agents depend on abstractions).
class ShadowService {
public:
    ShadowService() = default;
    ~ShadowService() { stopAll(); }

    void registerAgent(std::unique_ptr<Agent> agent) {
        const std::string id = agent->id;
        channel_.subscribe(id);
        runners_[id] = std::make_unique<AgentRunner>(*agent, channel_);
        agents_[id]  = std::move(agent);
    }

    void startAll() {
        for (auto& kv : agents_) {
            kv.second->onStart();
            runners_[kv.first]->start();
        }
    }

    void stopAll() {
        for (auto& kv : runners_) kv.second->stop();
        runners_.clear();
    }

    // Accessors for building agents before registration
    IChannel&        channel() { return channel_; }
    IMemoryStore&    memory()  { return memory_; }
    IKnowledgeStore& rdf()     { return rdf_; }

private:
    Channel       channel_;
    MemoryManager memory_;
    RDFStore      rdf_;

    std::unordered_map<std::string, std::unique_ptr<Agent>>       agents_;
    std::unordered_map<std::string, std::unique_ptr<AgentRunner>> runners_;
};
