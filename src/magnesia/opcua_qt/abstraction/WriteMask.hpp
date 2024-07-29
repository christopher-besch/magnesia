#pragma once

#include <cstdint>
#include <type_traits>

#include <open62541pp/Bitmask.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Indicates which attributes of a node a writeable.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.60
     */
    enum class WriteMask : uint32_t {
        NONE                      = 0U,
        ACCESS_LEVEL              = 1U << 0U,
        ARRAY_DIMENSIONS          = 1U << 1U,
        BROWSE_NAME               = 1U << 2U,
        CONTAINS_NO_LOOPS         = 1U << 3U,
        DATA_TYPE                 = 1U << 4U,
        DESCRIPTION               = 1U << 5U,
        DISPLAY_NAME              = 1U << 6U,
        EVENT_NOTIFIER            = 1U << 7U,
        EXECUTABLE                = 1U << 8U,
        HISTORIZING               = 1U << 9U,
        INVERSE_NAME              = 1U << 10U,
        IS_ABSTRACT               = 1U << 11U,
        MINIMUM_SAMPLING_INTERVAL = 1U << 12U,
        NODE_CLASS                = 1U << 13U,
        NODE_ID                   = 1U << 14U,
        SYMMETRIC                 = 1U << 15U,
        USER_ACCESS_LEVEL         = 1U << 16U,
        USER_EXECUTABLE           = 1U << 17U,
        USER_WRITE_MASK           = 1U << 18U,
        VALUE_RANK                = 1U << 19U,
        WRITE_MASK                = 1U << 20U,
        VALUE_FOR_VARIABLE_TYPE   = 1U << 21U,
        DATA_TYPE_DEFINITION      = 1U << 22U,
        ROLE_PERMISSIONS          = 1U << 23U,
        ACCESS_RESTRICTIONS       = 1U << 24U,
        ACCESS_LEVEL_EX           = 1U << 25U,

    };
} // namespace magnesia::opcua_qt::abstraction
