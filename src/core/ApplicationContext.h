#pragma once

#include <memory>

#include "CommandDispatcher.h"
#include "EventBus.h"
#include "Logger.h"
#include "ServiceRegistry.h"
#include "TaskScheduler.h"

namespace webide {
struct ApplicationContext {
    ServiceRegistry services;
    EventBus eventBus;
    CommandDispatcher commandDispatcher;
    TaskScheduler taskScheduler;
    Logger logger;
};
}  // namespace webide
