#pragma once

#include "LogEntry.hpp"
#include "abstraction/LogLevel.hpp"

#include <open62541pp/Logger.h>

#include <QList>
#include <QObject>
#include <QSet>
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
        void log(const LogEntry& entry) noexcept;
        /**
         * @brief returns a list of entries that match the log level
         *
         * @param levels
         *
         * @return QList of LogEntry
         */
        [[nodiscard]] QList<LogEntry> getLogForLevel(const QSet<LogLevel>& levels) const noexcept;

        opcua::Logger getOPCUALogger() noexcept;

      private:
        QList<LogEntry> m_log_entries;

      signals:
        void logEntryAdded(const LogEntry& entry);
    };
} // namespace magnesia::opcua_qt
