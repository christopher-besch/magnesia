#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * All NodeClasses.
     *
     * @see Node
     * @see VariableNode
     * @see VariableTypeNode
     * @see ObjectNode
     * @see ObjectTypeNode
     * @see ReferenceTypeNode
     * @see DataTypeNode
     * @see MethodNode
     * @see ViewNode
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.29
     */
    enum class NodeClass : int32_t {
        OBJECT         = 0b00000001,
        VARIABLE       = 0b00000010,
        METHOD         = 0b00000100,
        OBJECT_TYPE    = 0b00001000,
        VARIABLE_TYPE  = 0b00010000,
        REFERENCE_TYPE = 0b00100000,
        DATA_TYPE      = 0b01000000,
        VIEW           = 0b10000000,
    };
} // namespace magnesia::opcua_qt::abstraction
