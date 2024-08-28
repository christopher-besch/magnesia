#include "Logger.hpp"

#include "LogEntry.hpp"
#include "abstraction/LogLevel.hpp"

#include <ranges>
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
        auto filtered_entries = std::views::filter(
            m_log_entries, [&levels](const LogEntry& entry) { return levels.contains(entry.getLevel()); });
        return {filtered_entries.begin(), filtered_entries.end()};
    }

    opcua::Logger Logger::getOPCUALogger() noexcept {
        return [this](opcua::LogLevel log_level, opcua::LogCategory log_category, std::string_view message) {
            log({log_level, log_category, message});
        };
    }
} // namespace magnesia::opcua_qt
