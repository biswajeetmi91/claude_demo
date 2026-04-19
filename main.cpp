#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "shadow_service.hpp"

// Forwards every received message to Agent B
class AgentA : public Agent {
public:
    using Agent::Agent;

    void onStart() override {
        std::cout << "[Agent A] online — waiting for your messages\n";
    }

    void handleMessage(const Message& msg) override {
        if (msg.topic == "user_input") {
            std::cout << "[Agent A] sending to B: " << msg.payload << "\n";
            send("B", "msg", msg.payload);
        }
    }
};

// Prints everything it receives
class AgentB : public Agent {
public:
    using Agent::Agent;

    void onStart() override {
        std::cout << "[Agent B] online — ready to receive\n";
    }

    void handleMessage(const Message& msg) override {
        std::cout << "[Agent B] received from " << msg.from << ": " << msg.payload << "\n";
    }
};

int main() {
    ShadowService service;

    service.registerAgent(std::make_unique<AgentA>(
        "A", "Agent A", service.channel(), service.memory()));
    service.registerAgent(std::make_unique<AgentB>(
        "B", "Agent B", service.channel(), service.memory()));

    service.startAll();

    std::cout << "Type a message and press Enter to send (Ctrl+D to quit):\n\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        service.channel().publish({"terminal", "A", "user_input", line});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    service.stopAll();
    return 0;
}
