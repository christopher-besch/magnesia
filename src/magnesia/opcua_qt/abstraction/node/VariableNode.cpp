#include "VariableNode.hpp"

#include "../../../qt_version_check.hpp"
#include "../AccessLevelBitmask.hpp"
#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>
#include <open62541pp/Span.h>

#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    VariableNode::VariableNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<DataValue> VariableNode::getDataValue() {
        if (const auto& cache = getCacheDataValue(); cache.has_value()) {
            return cache;
        }

        return setCacheDataValue(DataValue(handle().readDataValue()));
    }

    std::optional<NodeId> VariableNode::getDataType() {
        return NodeId(handle().readDataType());
    }

    std::optional<ValueRank> VariableNode::getValueRank() {
        return static_cast<ValueRank>(handle().readValueRank());
    }

    std::optional<std::vector<quint32>> VariableNode::getArrayDimensions() {
        try {
            return handle().readArrayDimensions();
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<AccessLevelBitmask> VariableNode::getAccessLevel() {
        return AccessLevelBitmask(handle().readAccessLevel());
    }

    std::optional<AccessLevelBitmask> VariableNode::getUserAccessLevel() {
        return AccessLevelBitmask(handle().readUserAccessLevel());
    }

    std::optional<double> VariableNode::getMinimumSamplingInterval() {
        try {
            return handle().readMinimumSamplingInterval();
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> VariableNode::isHistorizing() {
        return handle().readHistorizing();
    }

    void VariableNode::setDataValue(DataValue& value) {
        handle().writeDataValue(value.handle());
    }

    void VariableNode::setDataValue(Variant& value) {
        handle().writeValue(value.handle());
    }

    void VariableNode::setDataType(NodeId& data_type) {
        handle().writeDataType(data_type.handle());
    }

    void VariableNode::setValueRank(ValueRank rank) {
        handle().writeValueRank(static_cast<opcua::ValueRank>(rank));
    }

    void VariableNode::setArrayDimensions(std::vector<quint32>& dimensions) {
        handle().writeArrayDimensions(dimensions);
    }

    void VariableNode::setAccessLevel(AccessLevelBitmask mask) {
        handle().writeAccessLevel(mask.handle());
    }

    void VariableNode::setUserAccessLevel(AccessLevelBitmask mask) {
        handle().writeUserAccessLevel(mask.handle());
    }

    void VariableNode::setHistorizing(bool historizing) {
        handle().writeHistorizing(historizing);
    }
} // namespace magnesia::opcua_qt::abstraction
