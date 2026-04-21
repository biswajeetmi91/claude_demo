#pragma once
#include <string>
#include "interfaces.hpp"
#include "rdf_store.hpp"

// --- Capability: send/broadcast messages (depends on IChannel) ---
class IMessagable {
public:
    explicit IMessagable(const std::string& id, IChannel& channel)
        : agentId_(id), channel_(channel) {}

    void send(const std::string& to, const std::string& topic, const std::string& payload) {
        channel_.publish({agentId_, to, topic, payload});
    }

    void broadcast(const std::string& topic, const std::string& payload) {
        channel_.publish({agentId_, "", topic, payload});
    }

private:
    const std::string& agentId_;
    IChannel& channel_;
};

// --- Capability: key-value memory (depends on IMemoryStore) ---
class IMemoryCapable {
public:
    explicit IMemoryCapable(const std::string& id, IMemoryStore& memory)
        : agentId_(id), memory_(memory) {}

    void remember(const std::string& key, const std::string& value) {
        memory_.set(agentId_, key, value);
    }

    std::optional<std::string> recall(const std::string& key) {
        return memory_.get(agentId_, key);
    }

private:
    const std::string& agentId_;
    IMemoryStore& memory_;
};

// --- Capability: RDF knowledge store (depends on IKnowledgeStore) ---
class IKnowledgeCapable {
public:
    explicit IKnowledgeCapable(IKnowledgeStore& rdf) : rdf_(rdf) {}

    void addTriple(const std::string& subject, const std::string& predicate,
                   const std::string& object) {
        rdf_.add(subject, predicate, object);
    }

    std::vector<Triple> queryTriples(const std::string& subject,
                                     const std::string& predicate,
                                     const std::string& object) const {
        return rdf_.query(subject, predicate, object);
    }

    std::optional<std::string> getTripleObject(const std::string& subject,
                                               const std::string& predicate) const {
        return rdf_.getObject(subject, predicate);
    }

protected:
    IKnowledgeStore& rdf_;
};

// --- Base Agent: composes all three capabilities ---
// Subclasses implement handleMessage() — the single responsibility of an agent.
class Agent : public IMessagable, public IMemoryCapable, public IKnowledgeCapable {
public:
    Agent(std::string id, std::string name,
          IChannel& channel, IMemoryStore& memory, IKnowledgeStore& rdf)
        : IMessagable(id, channel)
        , IMemoryCapable(id, memory)
        , IKnowledgeCapable(rdf)
        , id(std::move(id))
        , name(std::move(name)) {}

    virtual ~Agent() = default;

    // Contract: handle one incoming message; must not block indefinitely.
    virtual void handleMessage(const Message& msg) = 0;

    // Called once before the agent's message loop starts.
    virtual void onStart() {}

    const std::string id;
    const std::string name;
};
