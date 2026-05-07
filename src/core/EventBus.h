#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QVariantMap>

namespace webide {
class EventBus {
public:
    using Handler = std::function<void(const QVariantMap&)>;

    void subscribe(const std::string& topic, Handler handler) {
        subscriptions_[topic].push_back(std::move(handler));
    }

    void publish(const std::string& topic, const QVariantMap& payload = {}) const {
        const auto it = subscriptions_.find(topic);
        if (it == subscriptions_.end()) {
            return;
        }

        for (const auto& handler : it->second) {
            handler(payload);
        }
    }

private:
    std::unordered_map<std::string, std::vector<Handler>> subscriptions_;
};
}  // namespace webide
