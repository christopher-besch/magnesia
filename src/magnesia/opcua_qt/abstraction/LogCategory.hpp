#pragma once

namespace magnesia::opcua_qt {
    /*
     * The category of a log entry.
     *
     * @see LogEntry
     */
    enum class LogCategory {
        NETWORK,
        SECURE_CHANNEL,
        SESSION,
        SERVER,
        CLIENT,
        USERLAND,
        SECURITY_POLYCY,
        PANEL,
        ACTIVITY,
        APPLICATION,
    };
} // namespace magnesia::opcua_qt
