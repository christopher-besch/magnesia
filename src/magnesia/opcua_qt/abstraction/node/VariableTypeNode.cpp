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

    std::optional<DataValue> VariableTypeNode::getDataValue() {
        if (const auto& cache = getCacheDataValue(); cache.has_value()) {
            return cache;
        }

        try {
            return setCacheDataValue(DataValue(handle().readDataValue()));
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<NodeId> VariableTypeNode::getDataType() {
        return NodeId(handle().readDataType());
    }

    std::optional<ValueRank> VariableTypeNode::getValueRank() {
        return static_cast<ValueRank>(handle().readValueRank());
    }

    std::optional<std::vector<std::uint32_t>> VariableTypeNode::getArrayDimensions() {
        try {
            return handle().readArrayDimensions();
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> VariableTypeNode::isAbstract() {
        return handle().readIsAbstract();
    }

    void VariableTypeNode::setDataValue(const DataValue& value) {
        handle().writeDataValue(value.handle());
    }

    void VariableTypeNode::setDataValue(const Variant& value) {
        handle().writeValue(value.handle());
    }

    void VariableTypeNode::setDataType(const NodeId& data_type) {
        handle().writeDataType(data_type.handle());
    }

    void VariableTypeNode::setValueRank(ValueRank rank) {
        handle().writeValueRank(static_cast<opcua::ValueRank>(rank));
    }

    void VariableTypeNode::setArrayDimensions(const std::vector<std::uint32_t>& dimensions) {
        handle().writeArrayDimensions(dimensions);
    }

    void VariableTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
