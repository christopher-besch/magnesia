#include "VariableNode.hpp"

#include "Node.hpp"
#include "opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "opcua_qt/abstraction/DataValue.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"
#include "opcua_qt/abstraction/Variant.hpp"
#include "qt_version_check.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Node.h>
#include <open62541pp/Span.h>

#include <QList>
#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    VariableNode::VariableNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::Variable);
    }

    std::optional<DataValue> VariableNode::getDataValue() {
        return DataValue(handle().readDataValue());
    }

    std::optional<NodeId> VariableNode::getDataType() {
        return NodeId(handle().readDataType());
    }

    std::optional<ValueRank> VariableNode::getValueRank() {
        return static_cast<ValueRank>(handle().readValueRank());
    }

    std::optional<QList<quint32>> VariableNode::getArrayDimensions() {
        auto vector = handle().readArrayDimensions();
        auto list   = QList<quint32>();

        list.reserve(static_cast<qsizetype>(vector.size()));
        std::copy(vector.begin(), vector.end(), std::back_inserter(list));

        return list;
    }

    std::optional<AccessLevelBitmask> VariableNode::getAccessLevel() {
        return AccessLevelBitmask(handle().readAccessLevel());
    }

    std::optional<AccessLevelBitmask> VariableNode::getUserAccessLevel() {
        return AccessLevelBitmask(handle().readUserAccessLevel());
    }

    std::optional<double> VariableNode::getMinimumSamplingInterval() {
        return handle().readMinimumSamplingInterval();
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

    void VariableNode::setArrayDimensions(QList<quint32>& dimensions) {
        opcua::Span<const uint32_t> span(dimensions.data(), static_cast<std::size_t>(dimensions.size()));
        handle().writeArrayDimensions(span);
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
