#pragma once

#include <cstdint>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class EventNotifier
     * @brief Indicates if a node can be used to subscribe to events or read/write historic events.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.59
     */
    enum class EventNotifier : std::uint8_t {
        SUBSCRIBE_TO_EVENTS = 1,
        HISTORY_READ        = 4,
        HISTORY_WRITE       = 8,
    };

    inline QString event_notifier_to_string(EventNotifier event_notifier) {
        switch (event_notifier) {
            case EventNotifier::SUBSCRIBE_TO_EVENTS:
                return "Subscribe to events";
            case EventNotifier::HISTORY_READ:
                return "History read";
            case EventNotifier::HISTORY_WRITE:
                return "History Write";
            default:
                return "";
        }
    }
} // namespace magnesia::opcua_qt::abstraction
