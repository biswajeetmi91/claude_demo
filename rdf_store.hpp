#pragma once
#include <vector>
#include <mutex>
#include <algorithm>
#include "interfaces.hpp"

struct Triple {
    std::string subject;
    std::string predicate;
    std::string object;
};

class RDFStore : public IKnowledgeStore {
public:
    void add(const std::string& subject, const std::string& predicate,
             const std::string& object) override {
        std::lock_guard<std::mutex> lock(mutex_);
        triples_.push_back({subject, predicate, object});
    }

    // Wildcard: pass "" to match anything in that position
    std::vector<Triple> query(const std::string& subject,
                              const std::string& predicate,
                              const std::string& object) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Triple> results;
        for (const auto& t : triples_) {
            if ((subject.empty()   || t.subject   == subject)   &&
                (predicate.empty() || t.predicate == predicate) &&
                (object.empty()    || t.object    == object)) {
                results.push_back(t);
            }
        }
        return results;
    }

    std::optional<std::string> getObject(const std::string& subject,
                                         const std::string& predicate) const override {
        auto results = query(subject, predicate, "");
        if (results.empty()) return std::nullopt;
        return results.front().object;
    }

    void remove(const std::string& subject, const std::string& predicate,
                const std::string& object) override {
        std::lock_guard<std::mutex> lock(mutex_);
        triples_.erase(std::remove_if(triples_.begin(), triples_.end(),
            [&](const Triple& t) {
                return (subject.empty()   || t.subject   == subject)   &&
                       (predicate.empty() || t.predicate == predicate) &&
                       (object.empty()    || t.object    == object);
            }), triples_.end());
    }

private:
    std::vector<Triple> triples_;
    mutable std::mutex mutex_;
};
