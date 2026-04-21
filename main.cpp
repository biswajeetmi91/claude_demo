#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "shadow_service.hpp"
#include "persona_agent.hpp"

static std::unique_ptr<PersonaAgent> makePersona(
    const std::string& id,
    const std::string& displayName,
    const std::string& role,
    const std::string& greeting,
    std::vector<std::string> traits,
    ShadowService& service)
{
    auto agent = std::make_unique<PersonaAgent>(
        id, displayName, service.channel(), service.memory(), service.rdf());

    agent->defineTrait("name",     displayName);
    agent->defineTrait("role",     role);
    agent->defineTrait("greeting", greeting);
    for (auto& t : traits) agent->defineTrait("trait", t);

    // OCP: register topic handlers without modifying PersonaAgent
    agent->addTopicHandler("user_input", [a = agent.get()](const Message& msg) {
        a->printReceived(msg);
        a->replyInPersona(msg);
    });
    agent->addTopicHandler("chat", [a = agent.get()](const Message& msg) {
        a->printReceived(msg);
    });

    return agent;
}

int main() {
    ShadowService service;

    auto alice = makePersona("alice", "Alice", "mentor",
        "Hello! I'm Alice, your mentor. Ready to guide you.",
        {"patient", "encouraging"}, service);

    auto rex = makePersona("rex", "Rex", "critic",
        "Rex here. I'll tell you exactly what I think.",
        {"blunt", "analytical"}, service);

    service.registerAgent(std::move(alice));
    service.registerAgent(std::move(rex));

    service.startAll();

    std::cout << "\nType:  <agent_id> <message>  to send a message.\n";
    std::cout << "       e.g.  alice Hello Rex!\n";
    std::cout << "       e.g.  rex   Your feedback is harsh.\n";
    std::cout << "Press Ctrl+D to quit.\n\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        auto space = line.find(' ');
        if (space == std::string::npos) {
            std::cout << "Usage: <agent_id> <message>\n";
            continue;
        }
        service.channel().publish({"terminal", line.substr(0, space), "user_input", line.substr(space + 1)});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    service.stopAll();
    return 0;
}
