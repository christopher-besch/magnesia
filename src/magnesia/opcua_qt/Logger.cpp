#include "Logger.hpp"

#include "LogEntry.hpp"

#include <string_view>
#include <utility>
#include <vector>

#include <open62541pp/Logger.h>

#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    Logger::Logger(QObject* parent) : QObject(parent) {}

    void Logger::log(LogEntry entry) noexcept {
        m_log_entries.push_back(std::move(entry));
        Q_EMIT logEntryAdded(m_log_entries.size() - 1);
    }

    opcua::Logger Logger::getOPCUALogger() noexcept {
        return [this](opcua::LogLevel log_level, opcua::LogCategory log_category, std::string_view message) {
            log({log_level, log_category, message});
        };
    }
} // namespace magnesia::opcua_qt
