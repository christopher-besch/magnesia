#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Indicates how the value of a variable can be accessed (read/write) and if it contains current and/or historic
     * data.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/8.57
     */
    enum class AccessLevel : uint8_t {
        NONE,
        CURRENT_READ    = 1,
        CURRENT_WRITE   = 2,
        HISTORY_READ    = 4,
        HISTORY_WRITE   = 8,
        SEMANTIC_CHANGE = 16,
        STATUS_WRITE    = 32,
        TIMESTAMP_WRITE = 64,
    };
} // namespace magnesia::opcua_qt::abstraction
