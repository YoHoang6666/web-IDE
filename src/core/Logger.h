#pragma once

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QTextStream>

namespace webide {
enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    Logger() = default;
    explicit Logger(QString component) : component_(std::move(component)) {}

    static Logger& global() {
        static Logger instance(QStringLiteral("webide"));
        return instance;
    }

    static void configureGlobal(const QString& component, const QString& logFilePath = QString()) {
        auto& logger = global();
        logger.component_ = component;
        logger.logFilePath_ = logFilePath;
    }

    void setComponent(const QString& component) { component_ = component; }
    void setLogFilePath(const QString& path) { logFilePath_ = path; }
    void enableConsole(bool enabled) { consoleEnabled_ = enabled; }

    void debug(const QString& message, const QString& category = QString()) const { log(LogLevel::Debug, message, category); }
    void info(const QString& message, const QString& category = QString()) const { log(LogLevel::Info, message, category); }
    void warning(const QString& message, const QString& category = QString()) const { log(LogLevel::Warning, message, category); }
    void error(const QString& message, const QString& category = QString()) const { log(LogLevel::Error, message, category); }

    void log(LogLevel level, const QString& message, const QString& category = QString()) const {
        const QString line = formatLine(level, message, category);
        if (consoleEnabled_) {
            writeConsoleLine(level, line);
        }
        writeFileLine(line);
    }

private:
    QString formatLine(LogLevel level, const QString& message, const QString& category) const {
        const QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        const QString levelLabel = logLevelLabel(level);
        const QString categoryLabel = category.isEmpty() ? QStringLiteral("general") : category;
        const QString componentLabel = component_.isEmpty() ? QStringLiteral("webide") : component_;
        return QStringLiteral("[%1] [%2] [%3] [%4] %5")
            .arg(timestamp, componentLabel, categoryLabel, levelLabel, message);
    }

    static QString logLevelLabel(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:
                return QStringLiteral("DEBUG");
            case LogLevel::Info:
                return QStringLiteral("INFO");
            case LogLevel::Warning:
                return QStringLiteral("WARN");
            case LogLevel::Error:
                return QStringLiteral("ERROR");
        }
        return QStringLiteral("INFO");
    }

    void writeConsoleLine(LogLevel level, const QString& line) const {
        switch (level) {
            case LogLevel::Debug:
            case LogLevel::Info:
                qInfo().noquote() << line;
                break;
            case LogLevel::Warning:
                qWarning().noquote() << line;
                break;
            case LogLevel::Error:
                qCritical().noquote() << line;
                break;
        }
    }

    void writeFileLine(const QString& line) const {
        if (logFilePath_.isEmpty()) {
            return;
        }
        static QMutex mutex;
        QMutexLocker lock(&mutex);
        QFile file(logFilePath_);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            return;
        }
        QTextStream stream(&file);
        stream << line << '\n';
    }

    QString component_;
    QString logFilePath_;
    bool consoleEnabled_ = true;
};
}  // namespace webide
