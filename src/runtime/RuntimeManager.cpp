#include "RuntimeManager.h"

#include "LocalServerManager.h"
#include "NodeRuntime.h"
#include "PhpRuntime.h"
#include "ProcessManager.h"
#include "PythonRuntime.h"
#include "RuntimeBase.h"
#include "TerminalBridge.h"

namespace webide {
RuntimeManager::RuntimeManager(QObject* parent)
    : QObject(parent),
      processManager_(new ProcessManager(this)),
      localServerManager_(new LocalServerManager(this, processManager_, this)),
      terminalBridge_(new TerminalBridge(processManager_, this)) {
    registerRuntime(new PhpRuntime(this));
    registerRuntime(new NodeRuntime(this));
    registerRuntime(new PythonRuntime(this));
}

RuntimeManager::~RuntimeManager() = default;

void RuntimeManager::registerRuntime(RuntimeBase* runtime) {
    if (!runtime) {
        return;
    }

    runtimes_.push_back(runtime);
    connect(runtime, &RuntimeBase::environmentChanged, this, [this](const RuntimeEnvironment& environment) {
        environments_.insert(environment.id, environment);
        emit runtimeEnvironmentChanged(environment);
        emit runtimesChanged(environments());
    });
}

void RuntimeManager::detectRuntimes() {
    for (RuntimeBase* runtime : runtimes_) {
        runtime->detect();
    }
    emit runtimesChanged(environments());
}

QList<RuntimeEnvironment> RuntimeManager::environments() const { return environments_.values(); }

RuntimeEnvironment RuntimeManager::environment(const QString& runtimeId) const { return environments_.value(runtimeId); }

ProcessManager* RuntimeManager::processManager() const { return processManager_; }

LocalServerManager* RuntimeManager::localServerManager() const { return localServerManager_; }

TerminalBridge* RuntimeManager::terminalBridge() const { return terminalBridge_; }
}  // namespace webide

