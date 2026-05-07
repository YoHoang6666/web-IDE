#include "ScriptExecutionContext.h"

namespace webide {
ScriptExecutionContext::ScriptExecutionContext(QObject* parent) : QObject(parent) {}

void ScriptExecutionContext::setWorkspacePath(const QString& workspacePath) { workspacePath_ = workspacePath; }

QString ScriptExecutionContext::workspacePath() const { return workspacePath_; }
}  // namespace webide
