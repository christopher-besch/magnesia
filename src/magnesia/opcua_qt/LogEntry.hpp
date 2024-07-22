#pragma once

#include "abstraction/LogCategory.hpp"
#include "abstraction/LogLevel.hpp"

#include <string_view>

#include <open62541pp/Logger.h>

#include <QString>

namespace magnesia::opcua_qt {
    /**
     * @class LogEntry
     * @brief Entry for the logging system
     *
     */
    class LogEntry {
      public:
        /**
         * @brief Creates a LogEntry
         *
         * @param level a LogLevel
         * @param category a LogCategory
         * @param message a QString
         */
        LogEntry(LogLevel level, LogCategory category, QString message);
        /**
         * @brief Creates a LogEntry from objects from open62541pp
         *
         * @param level a LogLevel
         * @param category a LogCategory
         * @param message a QString
         */
        LogEntry(opcua::LogLevel level, opcua::LogCategory category, const std::string_view& message);
        /**
         * @brief Returns the log level
         *
         * @return a LogLevel
         */
        [[nodiscard]] LogLevel getLevel() const noexcept;
        /**
         * @brief Returns the log category
         *
         * @return a LogCategory
         */
        [[nodiscard]] LogCategory getCategory() const noexcept;
        /**
         * @brief Returns the message
         *
         * @return a QString
         */
        [[nodiscard]] QString getMessage() const noexcept;

      private:
        QString     m_message;
        LogLevel    m_level;
        LogCategory m_category;
    };
} // namespace magnesia::opcua_qt
