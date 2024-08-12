#pragma once

#include <cstdint>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Indicates how the value of a variable can be accessed (read/write) and if it contains current and/or historic
     * data.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/8.57
     */
    enum class AccessLevel : uint8_t {
        NONE            = 0,
        CURRENT_READ    = 1,
        CURRENT_WRITE   = 2,
        HISTORY_READ    = 4,
        HISTORY_WRITE   = 8,
        SEMANTIC_CHANGE = 16,
        STATUS_WRITE    = 32,
        TIMESTAMP_WRITE = 64,
    };

    inline QString access_level_to_string(AccessLevel access_level) {
        switch (access_level) {
            case AccessLevel::NONE:
                return "None";
            case AccessLevel::CURRENT_READ:
                return "Current Read";
            case AccessLevel::CURRENT_WRITE:
                return "Current Write";
            case AccessLevel::HISTORY_READ:
                return "History Read";
            case AccessLevel::HISTORY_WRITE:
                return "History Write";
            case AccessLevel::SEMANTIC_CHANGE:
                return "Semantic Change";
            case AccessLevel::STATUS_WRITE:
                return "Status Write";
            case AccessLevel::TIMESTAMP_WRITE:
                return "Timestamp Write";
            default:
                return "";
        }
    }
} // namespace magnesia::opcua_qt::abstraction
