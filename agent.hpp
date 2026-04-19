#pragma once
#include <string>
#include "message.hpp"
#include "channel.hpp"
#include "memory.hpp"

class Agent {
public:
    Agent(std::string id, std::string name, Channel& channel, MemoryManager& memory)
        : id(std::move(id)), name(std::move(name)), channel_(channel), memory_(memory) {}

    virtual ~Agent() = default;

    virtual void handleMessage(const Message& msg) = 0;

    // Called once when the agent starts inside the shadow service
    virtual void onStart() {}

    void send(const std::string& to, const std::string& topic, const std::string& payload) {
        channel_.publish({id, to, topic, payload});
    }

    void broadcast(const std::string& topic, const std::string& payload) {
        channel_.publish({id, "", topic, payload});
    }

    void remember(const std::string& key, const std::string& value) {
        memory_.set(id, key, value);
    }

    std::optional<std::string> recall(const std::string& key) {
        return memory_.get(id, key);
    }

    const std::string id;
    const std::string name;

protected:
    Channel& channel_;
    MemoryManager& memory_;
};
