#include "VariableTypeNode.hpp"

#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "Node.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>
#include <open62541pp/Span.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    VariableTypeNode::VariableTypeNode(opcua::Node<opcua::Client> node, QObject* parent)
        : Node(std::move(node), parent) {}

    const DataValue* VariableTypeNode::getDataValue() {
        try {
            return &wrapCache(&Cache::data_value, [this] { return DataValue{handle().readDataValue()}; });
        } catch (opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<NodeId> VariableTypeNode::getDataType() {
        return wrapCache(&Cache::data_type, [this] { return NodeId{handle().readDataType()}; });
    }

    std::optional<ValueRank> VariableTypeNode::getValueRank() {
        return wrapCache(&Cache::value_rank, [this] { return static_cast<ValueRank>(handle().readValueRank()); });
    }

    const std::vector<std::uint32_t>* VariableTypeNode::getArrayDimensions() {
        try {
            return &wrapCache(&Cache::array_dimensions, [this] { return handle().readArrayDimensions(); });
        } catch (opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<bool> VariableTypeNode::isAbstract() {
        return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
    }

    void VariableTypeNode::setDataValue(const DataValue& value) {
        handle().writeDataValue(value.handle());
        invalidateCache(&Cache::data_value);
    }

    void VariableTypeNode::setDataValue(const Variant& value) {
        handle().writeValue(value.handle());
        invalidateCache(&Cache::data_value);
    }

    void VariableTypeNode::setDataType(const NodeId& data_type) {
        handle().writeDataType(data_type.handle());
        invalidateCache(&Cache::data_type);
    }

    void VariableTypeNode::setValueRank(ValueRank rank) {
        handle().writeValueRank(static_cast<opcua::ValueRank>(rank));
        invalidateCache(&Cache::value_rank);
    }

    void VariableTypeNode::setArrayDimensions(const std::vector<std::uint32_t>& dimensions) {
        handle().writeArrayDimensions(dimensions);
        invalidateCache(&Cache::array_dimensions);
    }

    void VariableTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
        invalidateCache(&Cache::is_abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
