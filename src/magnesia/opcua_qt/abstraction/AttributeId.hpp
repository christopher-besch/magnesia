#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class AttributeId
     * @brief Identifiers for node attributes.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/5.9
     */
    enum class AttributeId : int32_t {
        NODE_ID = 1,
        NODE_CLASS,
        BROWSE_NAME,
        DISPLAY_NAME,
        DESCRIPTION,
        WRITE_MASK,
        USER_WRITE_MASK,
        IS_ABSTRACT,
        SYMMETRIC,
        INVERSE_NAME,
        CONTAINS_NO_LOOPS,
        EVENT_NOTFIER,
        VALUE,
        DATA_TYPE,
        VALUE_RANK,
        ARRAY_DIMENSIONS,
        ACCESS_LEVEL,
        USER_ACCESS_LEVEL,
        MINIMUM_SAMPLING_INTERVAL,
        HISTORIZING,
        EXECUTABLE,
        USER_EXECUTABLE,
        DATA_TYPE_DEFINITION,
        ROLE_PERMISSIONS,
        USER_ROLE_PERMISSIONS,
        ACCESS_RESTRICTIONS,
        ACCESS_LEVEL_EX,
    };
} // namespace magnesia::opcua_qt::abstraction
