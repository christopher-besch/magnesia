#pragma once

#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    /*
     * The severity level of a log entry.
     *
     * @see LogEntry
     */
    enum class LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL,
    };

    inline QString log_level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE:
                return "Trace";
            case LogLevel::DEBUG:
                return "Debug";
            case LogLevel::INFO:
                return "Info";
            case LogLevel::WARNING:
                return "Warning";
            case LogLevel::ERROR:
                return "Error";
            case LogLevel::FATAL:
                return "Fatal";
        }
        return "not implemented";
    }
} // namespace magnesia::opcua_qt
