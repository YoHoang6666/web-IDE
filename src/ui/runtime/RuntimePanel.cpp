#include "RuntimePanel.h"

#include <QDesktopServices>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QUrl>
#include <QVBoxLayout>

#include "runtime/LocalServerManager.h"
#include "runtime/ProcessManager.h"
#include "runtime/RuntimeManager.h"
#include "runtime/TerminalBridge.h"
#include "ServerControlWidget.h"
#include "ProcessMonitorWidget.h"
#include "RuntimeSettingsWidget.h"

namespace webide {
RuntimePanel::RuntimePanel(RuntimeManager* runtimeManager, QWidget* parent)
    : QWidget(parent),
      runtimeManager_(runtimeManager),
      runtimesList_(new QListWidget(this)),
      serverControl_(new ServerControlWidget(this)),
      processMonitor_(new ProcessMonitorWidget(this)),
      runtimeSettings_(new RuntimeSettingsWidget(this)),
      logsOutput_(new QPlainTextEdit(this)) {
    setupUi();
    wireSignals();

    if (runtimeManager_) {
        runtimeManager_->detectRuntimes();
    }
    refreshRuntimeList();
    refreshProcessList();
}

void RuntimePanel::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    logsOutput_->setReadOnly(true);
    logsOutput_->setPlaceholderText(tr("Runtime logs"));

    auto* vertical = new QSplitter(Qt::Vertical, this);
    auto* top = new QWidget(vertical);
    auto* topLayout = new QVBoxLayout(top);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(runtimesList_);
    topLayout->addWidget(runtimeSettings_);
    topLayout->addWidget(serverControl_);

    vertical->addWidget(top);
    vertical->addWidget(processMonitor_);
    vertical->addWidget(logsOutput_);
    vertical->setStretchFactor(0, 2);
    vertical->setStretchFactor(1, 2);
    vertical->setStretchFactor(2, 1);
    layout->addWidget(vertical);
}

void RuntimePanel::wireSignals() {
    if (!runtimeManager_) {
        return;
    }

    connect(runtimeManager_, &RuntimeManager::runtimesChanged, this, [this](const QList<RuntimeEnvironment>&) {
        refreshRuntimeList();
    });

    connect(runtimeManager_->processManager(), &ProcessManager::processUpdated, this, [this](const ManagedProcess&) {
        refreshProcessList();
    });

    connect(runtimeManager_->processManager(), &ProcessManager::processOutput, this, [this](const QString& processId, const QString& text, bool isError) {
        if (text.isEmpty()) {
            return;
        }
        logsOutput_->appendPlainText(QStringLiteral("[%1] %2%3")
                                         .arg(processId, isError ? QStringLiteral("[err] ") : QString(), text.trimmed()));
    });

    connect(serverControl_, &ServerControlWidget::startRequested, this, [this](const QString& runtimeId, const QString& command, int port) {
        QString processId;
        if (runtimeId == QStringLiteral("php")) {
            processId = runtimeManager_->localServerManager()->startPhpServer({}, port);
        } else if (runtimeId == QStringLiteral("node")) {
            processId = runtimeManager_->localServerManager()->startNodeScript({}, command.isEmpty() ? QStringLiteral("index.js") : command, port);
        } else if (runtimeId == QStringLiteral("python")) {
            processId = runtimeManager_->localServerManager()->startPythonModule({}, command.isEmpty() ? QStringLiteral("http.server") : command, port);
        } else {
            processId = runtimeManager_->localServerManager()->startStaticServer({}, port);
        }
        if (!processId.isEmpty()) {
            activeProcessId_ = processId;
            emit statusMessage(tr("Started process: %1").arg(processId));
        }
    });

    connect(serverControl_, &ServerControlWidget::stopRequested, this, [this]() {
        const QString processId = processMonitor_->selectedProcessId().isEmpty() ? activeProcessId_ : processMonitor_->selectedProcessId();
        if (!processId.isEmpty()) {
            runtimeManager_->processManager()->stopProcess(processId);
        }
    });

    connect(serverControl_, &ServerControlWidget::restartRequested, this, [this]() {
        const QString processId = processMonitor_->selectedProcessId().isEmpty() ? activeProcessId_ : processMonitor_->selectedProcessId();
        if (!processId.isEmpty()) {
            runtimeManager_->processManager()->killProcess(processId);
        }
    });

    connect(serverControl_, &ServerControlWidget::killRequested, this, [this]() {
        const QString processId = processMonitor_->selectedProcessId().isEmpty() ? activeProcessId_ : processMonitor_->selectedProcessId();
        if (!processId.isEmpty()) {
            runtimeManager_->processManager()->killProcess(processId);
        }
    });

    connect(serverControl_, &ServerControlWidget::openBrowserRequested, this, [this](int port) {
        QDesktopServices::openUrl(QUrl(QStringLiteral("http://localhost:%1").arg(port)));
    });

    connect(serverControl_, &ServerControlWidget::openTerminalRequested, this, [this](const QString& command) {
        if (!command.isEmpty()) {
            runtimeManager_->terminalBridge()->runShellCommand(command);
        }
    });
}

void RuntimePanel::refreshRuntimeList() {
    runtimesList_->clear();
    if (!runtimeManager_) {
        return;
    }

    const QList<RuntimeEnvironment> environments = runtimeManager_->environments();
    for (const RuntimeEnvironment& environment : environments) {
        const QString line = QStringLiteral("%1  |  %2  |  %3")
                                 .arg(environment.displayName,
                                      environment.available ? tr("Installed") : tr("Not Found"),
                                      environment.version);
        runtimesList_->addItem(line);

        if (environment.id == QStringLiteral("php")) {
            runtimeSettings_->setPhpPath(environment.executablePath);
        } else if (environment.id == QStringLiteral("node")) {
            runtimeSettings_->setNodePath(environment.executablePath);
        } else if (environment.id == QStringLiteral("python")) {
            runtimeSettings_->setPythonPath(environment.executablePath);
        }
    }
}

void RuntimePanel::refreshProcessList() {
    if (!runtimeManager_) {
        return;
    }
    processMonitor_->setProcesses(runtimeManager_->processManager()->processes());
}
}  // namespace webide
