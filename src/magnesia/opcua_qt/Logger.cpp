#include "Logger.hpp"

#include "LogEntry.hpp"
#include "abstraction/LogLevel.hpp"

#include <set>
#include <string_view>
#include <vector>

#include <open62541pp/Logger.h>

#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    Logger::Logger(QObject* parent) : QObject(parent) {}

    void Logger::log(LogEntry entry) noexcept {
        m_log_entries.push_back(entry);
        Q_EMIT logEntryAdded(entry);
    }

    std::vector<LogEntry> Logger::getLogForLevel(const std::set<LogLevel>& levels) const noexcept {
        std::vector<LogEntry> output;
        for (const LogEntry& log : m_log_entries) {
            if (levels.contains(log.getLevel())) {
                output.push_back(log);
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
