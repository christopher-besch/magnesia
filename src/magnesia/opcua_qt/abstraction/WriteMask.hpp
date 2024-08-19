#pragma once

#include <cstdint>

#include <open62541pp/Common.h>

#include <QString>

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

    inline QString write_mask_to_string(WriteMask write_mask) {
        switch (write_mask) {
            case WriteMask::NONE:
                return "None";
            case WriteMask::ACCESS_LEVEL:
                return "AccessLevel";
            case WriteMask::ARRAY_DIMENSIONS:
                return "ArrayDimensions";
            case WriteMask::BROWSE_NAME:
                return "BrowseName";
            case WriteMask::CONTAINS_NO_LOOPS:
                return "ContainsNoLoops";
            case WriteMask::DATA_TYPE:
                return "DataType";
            case WriteMask::DESCRIPTION:
                return "Description";
            case WriteMask::DISPLAY_NAME:
                return "DisplayName";
            case WriteMask::EVENT_NOTIFIER:
                return "EventNotifier";
            case WriteMask::EXECUTABLE:
                return "Executable";
            case WriteMask::HISTORIZING:
                return "Historizing";
            case WriteMask::INVERSE_NAME:
                return "InverseName";
            case WriteMask::IS_ABSTRACT:
                return "IsAbstract";
            case WriteMask::MINIMUM_SAMPLING_INTERVAL:
                return "MinimumSamplingInterval";
            case WriteMask::NODE_CLASS:
                return "NodeClass";
            case WriteMask::NODE_ID:
                return "NodeId";
            case WriteMask::SYMMETRIC:
                return "Symmetric";
            case WriteMask::USER_ACCESS_LEVEL:
                return "UserAccessLevel";
            case WriteMask::USER_EXECUTABLE:
                return "UserExecutable";
            case WriteMask::USER_WRITE_MASK:
                return "UserWriteMask";
            case WriteMask::VALUE_RANK:
                return "ValueRank";
            case WriteMask::WRITE_MASK:
                return "WriteMask";
            case WriteMask::VALUE_FOR_VARIABLE_TYPE:
                return "ValueForVariableType";
            case WriteMask::DATA_TYPE_DEFINITION:
                return "DataTypeDefinition";
            case WriteMask::ROLE_PERMISSIONS:
                return "RolePermissions";
            case WriteMask::ACCESS_RESTRICTIONS:
                return "AccessRestrictions";
            case WriteMask::ACCESS_LEVEL_EX:
                return "AccessLevelEx";
            default:
                return "";
        }
    }
} // namespace magnesia::opcua_qt::abstraction
