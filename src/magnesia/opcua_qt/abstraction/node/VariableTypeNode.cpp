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
} // namespace magnesia::opcua_qt::abstraction
