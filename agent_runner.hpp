#pragma once
#include <thread>
#include <atomic>
#include <iostream>
#include "agent.hpp"
#include "interfaces.hpp"

// AgentRunner: single responsibility — run one agent in a background thread.
// It owns the message loop; ShadowService owns lifecycle orchestration.
class AgentRunner {
public:
    AgentRunner(Agent& agent, IChannel& channel)
        : agent_(agent), channel_(channel), running_(false) {}

    ~AgentRunner() { stop(); }

    void start() {
        running_ = true;
        thread_ = std::thread([this]() { loop(); });
    }

    void stop() {
        if (!running_.exchange(false)) return;
        // Unblock channel_.receive() with a poison-pill
        channel_.publish({"system", agent_.id, "__shutdown__", ""});
        if (thread_.joinable()) thread_.join();
    }

private:
    void loop() {
        while (running_) {
            Message msg = channel_.receive(agent_.id);
            if (msg.topic == "__shutdown__") break;
            try {
                agent_.handleMessage(msg);
            } catch (const std::exception& e) {
                std::cerr << "[AgentRunner] " << agent_.id << " error: " << e.what() << "\n";
            }
        }
    }

    Agent& agent_;
    IChannel& channel_;
    std::atomic<bool> running_;
    std::thread thread_;
};
