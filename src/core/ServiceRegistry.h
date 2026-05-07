#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace webide {
class ServiceRegistry {
public:
    template <typename T>
    void registerService(std::shared_ptr<T> service) {
        services_[std::type_index(typeid(T))] = std::move(service);
    }

    template <typename T>
    std::shared_ptr<T> resolve() const {
        const auto it = services_.find(std::type_index(typeid(T)));
        if (it == services_.end()) {
            throw std::runtime_error("Requested service is not registered");
        }
        return std::static_pointer_cast<T>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};
}  // namespace webide
