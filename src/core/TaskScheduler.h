#pragma once

#include <functional>

#include <QtConcurrent/QtConcurrent>

namespace webide {
class TaskScheduler {
public:
    template <typename Callable>
    void schedule(Callable&& callable) {
        QtConcurrent::run(std::forward<Callable>(callable));
    }
};
}  // namespace webide
