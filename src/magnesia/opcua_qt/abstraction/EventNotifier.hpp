#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Indicates if a node can be used to subscribe to events or read/write historic events.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.59
     */
    enum class EventNotifier : uint8_t {
        SUBSCRIBE_TO_EVENTS = 1,
        HISTORY_READ        = 4,
        HISTORY_WRITE       = 8,
    };
} // namespace magnesia::opcua_qt::abstraction
