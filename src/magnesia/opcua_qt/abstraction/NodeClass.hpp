#pragma once

#include <cstdint>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class NodeClass
     * @brief All NodeClasses as an Enumeration
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

    inline QString node_class_to_string(NodeClass node_class) {
        switch (node_class) {
            case NodeClass::OBJECT:
                return "Object";
            case NodeClass::VARIABLE:
                return "Variable";
            case NodeClass::METHOD:
                return "Method";
            case NodeClass::OBJECT_TYPE:
                return "Object Type";
            case NodeClass::VARIABLE_TYPE:
                return "Variable Type";
            case NodeClass::REFERENCE_TYPE:
                return "Reference Type";
            case NodeClass::DATA_TYPE:
                return "Data Type";
            case NodeClass::VIEW:
                return "View";
            default:
                return "<invalid>";
        }
    }
} // namespace magnesia::opcua_qt::abstraction
