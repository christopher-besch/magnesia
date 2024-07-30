#include "Logger.hpp"

#include "LogEntry.hpp"
#include "abstraction/LogLevel.hpp"

#include <string_view>

#include <open62541pp/Logger.h>

#include <QList>
#include <QObject>
#include <QSet>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    Logger::Logger(QObject* parent) : QObject(parent) {}

    void Logger::log(const LogEntry& entry) noexcept {
        m_log_entries.append(entry);
        Q_EMIT logEntryAdded(entry);
    }

    QList<LogEntry> Logger::getLogForLevel(const QSet<LogLevel>& levels) const noexcept {
        QList<LogEntry> output;
        for (const LogEntry& log : m_log_entries) {
            if (levels.contains(log.getLevel())) {
                output.append(log);
            }
        }
        return output;
    }

    opcua::Logger Logger::getOPCUALogger() noexcept {
        return [&](opcua::LogLevel log_level, opcua::LogCategory log_category, std::string_view message) {
            log({log_level, log_category, message});
        };
    }
} // namespace magnesia::opcua_qt
