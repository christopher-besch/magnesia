#pragma once

#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "Node.hpp"

#include <cstdint>
#include <optional>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class VariableTypeNode
     * @brief Subclass of Node for VariableType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.6.5
     */
    class VariableTypeNode : public Node {
      public:
        /**
         * @param node
         * @param parent Parent of the VariableTypeNode.
         */
        explicit VariableTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] const DataValue*                  getDataValue() override;
        [[nodiscard]] std::optional<NodeId>             getDataType() override;
        [[nodiscard]] std::optional<ValueRank>          getValueRank() override;
        [[nodiscard]] const std::vector<std::uint32_t>* getArrayDimensions() override;
        [[nodiscard]] std::optional<bool>               isAbstract() override;

        void setDataValue(const DataValue& value) override;
        void setDataValue(const Variant& value) override;
        void setDataType(const NodeId& data_type) override;
        void setValueRank(ValueRank rank) override;
        void setArrayDimensions(const std::vector<std::uint32_t>& dimensions) override;
        void setAbstract(bool abstract) override;
    };
} // namespace magnesia::opcua_qt::abstraction
