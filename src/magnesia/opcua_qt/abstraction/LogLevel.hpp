#pragma once

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
} // namespace magnesia::opcua_qt
