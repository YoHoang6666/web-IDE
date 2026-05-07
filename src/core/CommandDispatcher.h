#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <QVariantMap>

namespace webide {
class CommandDispatcher {
public:
    using CommandHandler = std::function<void(const QVariantMap&)>;

    void registerCommand(const std::string& command, CommandHandler handler) {
        handlers_[command] = std::move(handler);
    }

    bool dispatch(const std::string& command, const QVariantMap& payload = {}) const {
        const auto it = handlers_.find(command);
        if (it == handlers_.end()) {
            return false;
        }
        it->second(payload);
        return true;
    }

private:
    std::unordered_map<std::string, CommandHandler> handlers_;
};
}  // namespace webide
