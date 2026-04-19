#pragma once
#include <string>

struct Message {
    std::string from;
    std::string to;      // empty = broadcast
    std::string topic;
    std::string payload;
};
