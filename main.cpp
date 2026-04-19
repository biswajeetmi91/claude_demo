#include <iostream>
#include <thread>
#include <chrono>
#include "shadow_service.hpp"

// Agent that sends a greeting and remembers it
class GreeterAgent : public Agent {
public:
    using Agent::Agent;

    void onStart() override {
        remember("greeting", "hello");
        send("responder", "greet", recall("greeting").value());
    }

    void handleMessage(const Message& msg) override {
        std::cout << "[" << name << "] received '" << msg.payload
                  << "' from " << msg.from << "\n";
    }
};

// Agent that responds to greetings
class ResponderAgent : public Agent {
public:
    using Agent::Agent;

    void handleMessage(const Message& msg) override {
        if (msg.topic == "greet") {
            std::cout << "[" << name << "] got greeting: '" << msg.payload << "'\n";
            remember("last_greeter", msg.from);
            send(msg.from, "reply", "world");
        }
    }
};

int main() {
    ShadowService service;

    service.registerAgent(std::make_unique<GreeterAgent>(
        "greeter", "Greeter", service.channel(), service.memory()));
    service.registerAgent(std::make_unique<ResponderAgent>(
        "responder", "Responder", service.channel(), service.memory()));

    service.startAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    service.stopAll();

    // Show memory state
    auto lastGreeter = service.memory().get("responder", "last_greeter");
    std::cout << "[Memory] responder.last_greeter = "
              << lastGreeter.value_or("(none)") << "\n";
    return 0;
}
