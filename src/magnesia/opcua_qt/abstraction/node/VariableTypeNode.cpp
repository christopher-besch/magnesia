#include "VariableTypeNode.hpp"

#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "Node.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    VariableTypeNode::VariableTypeNode(opcua::Node<opcua::Client> node, QObject* parent)
        : Node(std::move(node), parent) {}

    const DataValue* VariableTypeNode::getDataValue() {
        try {
            return &wrapCache(&Cache::data_value, [this] { return DataValue{handle().readDataValue()}; });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<NodeId> VariableTypeNode::getDataType() {
        try {
            return wrapCache(&Cache::data_type, [this] { return NodeId{handle().readDataType()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<ValueRank> VariableTypeNode::getValueRank() {
        try {
            return wrapCache(&Cache::value_rank, [this] { return static_cast<ValueRank>(handle().readValueRank()); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    const std::vector<std::uint32_t>* VariableTypeNode::getArrayDimensions() {
        try {
            return &wrapCache(&Cache::array_dimensions, [this] { return handle().readArrayDimensions(); });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<bool> VariableTypeNode::isAbstract() {
        try {
            return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
