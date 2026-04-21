#pragma once
#include <string>
#include <vector>
#include <optional>
#include "message.hpp"

// --- Communication abstraction ---
struct IChannel {
    virtual ~IChannel() = default;
    virtual void subscribe(const std::string& agentId) = 0;
    virtual void publish(const Message& msg) = 0;
    virtual Message receive(const std::string& agentId) = 0;
    virtual bool tryReceive(const std::string& agentId, Message& out) = 0;
};

// --- Memory abstraction ---
struct IMemoryStore {
    virtual ~IMemoryStore() = default;
    virtual void set(const std::string& agentId, const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> get(const std::string& agentId, const std::string& key) = 0;
    virtual void clear(const std::string& agentId) = 0;
};

// --- RDF knowledge abstraction ---
struct Triple;   // forward declared; defined in rdf_store.hpp

struct IKnowledgeStore {
    virtual ~IKnowledgeStore() = default;
    virtual void add(const std::string& subject, const std::string& predicate, const std::string& object) = 0;
    virtual std::vector<Triple> query(const std::string& subject,
                                      const std::string& predicate,
                                      const std::string& object) const = 0;
    virtual std::optional<std::string> getObject(const std::string& subject,
                                                  const std::string& predicate) const = 0;
    virtual void remove(const std::string& subject, const std::string& predicate,
                        const std::string& object) = 0;
};
