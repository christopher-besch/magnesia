#pragma once

#include "LogEntry.hpp"

#include <cstddef>
#include <vector>

#include <open62541pp/Logger.h>

#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    /**
     * @class Logger
     * @brief Manages all LogEntrys that have been logged with the log function.
     */
    class Logger : public QObject {
        Q_OBJECT

      public:
        explicit Logger(QObject* parent = nullptr);
        /**
         * @brief Creates a log from a LogEntry
         *
         * @param entry a LogEntry
         */
        void log(LogEntry entry) noexcept;

        /**
         * Retrieves the OPC UA logger.
         */
        opcua::Logger getOPCUALogger() noexcept;

        /**
         * Provides access to all log entries.
         *
         * @return vector of all log entries
         */
        [[nodiscard]] const std::vector<LogEntry>& getEntries() const {
            return m_log_entries;
        }

      private:
        std::vector<LogEntry> m_log_entries;

      signals:
        void logEntryAdded(std::size_t index);
    };
} // namespace magnesia::opcua_qt
