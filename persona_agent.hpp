#pragma once
#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>
#include "agent.hpp"

// PersonaAgent: an agent whose identity is defined by RDF triples.
//
// Persona triples (all keyed by agent id):
//   (id, "name",     "Alice")
//   (id, "role",     "mentor")
//   (id, "trait",    "patient")      <- multiple allowed
//   (id, "greeting", "Hello!")
//
// OCP: topic handlers are registered via addTopicHandler(), so new message
// types can be supported by callers without modifying this class.

class PersonaAgent : public Agent {
public:
    using Agent::Agent;
    using TopicHandler = std::function<void(const Message&)>;

    void defineTrait(const std::string& predicate, const std::string& value) {
        rdf_.add(id, predicate, value);
    }

    // Register a handler for a message topic (OCP: open for extension)
    void addTopicHandler(const std::string& topic, TopicHandler handler) {
        handlers_[topic] = std::move(handler);
    }

    void onStart() override {
        auto nameVal  = rdf_.getObject(id, "name").value_or(id);
        auto roleVal  = rdf_.getObject(id, "role").value_or("agent");
        auto greeting = rdf_.getObject(id, "greeting").value_or("Hello.");
        std::cout << "[" << nameVal << " / " << roleVal << "] " << greeting << "\n";
    }

    // Contract: dispatch to the registered handler for msg.topic, if any.
    // Unknown topics are silently ignored — no side effects, no exceptions.
    void handleMessage(const Message& msg) override {
        auto it = handlers_.find(msg.topic);
        if (it != handlers_.end()) {
            it->second(msg);
        }
    }

    // Helpers available to external topic handlers
    void printReceived(const Message& msg) const {
        auto nameVal = rdf_.getObject(id, "name").value_or(id);
        auto roleVal = rdf_.getObject(id, "role").value_or("agent");

        auto traits = rdf_.query(id, "trait", "");
        std::ostringstream traitStr;
        for (size_t i = 0; i < traits.size(); ++i) {
            if (i > 0) traitStr << ", ";
            traitStr << traits[i].object;
        }

        std::cout << "[" << nameVal << " / " << roleVal << "]"
                  << " (traits: " << traitStr.str() << ")"
                  << " received: \"" << msg.payload << "\""
                  << " from " << msg.from << "\n";
    }

public:
    void replyInPersona(const Message& msg) {
        auto nameVal = rdf_.getObject(id, "name").value_or(id);
        send(msg.from, "chat", "[" + nameVal + "] I hear you: \"" + msg.payload + "\"");
    }

private:
    std::unordered_map<std::string, TopicHandler> handlers_;
};
