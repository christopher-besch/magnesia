#include "LogEntry.hpp"

#include "abstraction/LogCategory.hpp"
#include "abstraction/LogLevel.hpp"

#include <string>
#include <string_view>
#include <utility>

#include <open62541pp/Logger.h>

#include <QString>

namespace magnesia::opcua_qt {
    LogEntry::LogEntry(LogLevel level, LogCategory category, QString message)
        : m_message(std::move(message)), m_level(level), m_category(category) {}

    LogEntry::LogEntry(opcua::LogLevel level, opcua::LogCategory category, const std::string_view& message)
        : m_message(QString::fromStdString(std::string(message))), m_level(static_cast<LogLevel>(level)),
          m_category(static_cast<LogCategory>(category)) {}

    LogLevel LogEntry::getLevel() const noexcept {
        return m_level;
    }

    LogCategory LogEntry::getCategory() const noexcept {
        return m_category;
    }

    const QString& LogEntry::getMessage() const noexcept {
        return m_message;
    }
} // namespace magnesia::opcua_qt
