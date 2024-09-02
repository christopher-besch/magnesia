#pragma once

#include <QString>

namespace magnesia::opcua_qt {
    /**
     * @class LogCategory
     * @brief The category of a log entry.
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

    inline QString log_category_to_string(LogCategory category) {
        switch (category) {
            case LogCategory::NETWORK:
                return "Network";
            case LogCategory::SECURE_CHANNEL:
                return "Secure Channel";
            case LogCategory::SESSION:
                return "Session";
            case LogCategory::SERVER:
                return "Server";
            case LogCategory::CLIENT:
                return "Client";
            case LogCategory::USERLAND:
                return "Userland";
            case LogCategory::SECURITY_POLYCY:
                return "Security Policy";
            case LogCategory::PANEL:
                return "Panel";
            case LogCategory::ACTIVITY:
                return "Activity";
            case LogCategory::APPLICATION:
                return "Application";
        }
        return "not implemented";
    }
} // namespace magnesia::opcua_qt
