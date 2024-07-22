#include "VariableTypeNode.hpp"

#include "Node.hpp"
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
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    VariableTypeNode::VariableTypeNode(opcua::Node<opcua::Client> node) : Node(std::move(node)) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::VariableType);
    }

    std::optional<DataValue> VariableTypeNode::getDataValue() {
        return DataValue(handle().readDataValue());
    }

    std::optional<NodeId> VariableTypeNode::getDataType() {
        return NodeId(handle().readDataType());
    }

    std::optional<ValueRank> VariableTypeNode::getValueRank() {
        return static_cast<ValueRank>(handle().readValueRank());
    }

    std::optional<QList<quint32>> VariableTypeNode::getArrayDimensions() {
        auto vector = handle().readArrayDimensions();
        auto list   = QList<quint32>();

        list.reserve(static_cast<qsizetype>(vector.size()));
        std::copy(vector.begin(), vector.end(), std::back_inserter(list));

        return list;
    }

    std::optional<bool> VariableTypeNode::isAbstract() {
        return handle().readIsAbstract();
    }

    void VariableTypeNode::setDataValue(DataValue& value) {
        handle().writeDataValue(value.handle());
    }

    void VariableTypeNode::setDataValue(Variant& value) {
        handle().writeValue(value.handle());
    }

    void VariableTypeNode::setDataType(NodeId& data_type) {
        handle().writeDataType(data_type.handle());
    }

    void VariableTypeNode::setValueRank(ValueRank rank) {
        handle().writeValueRank(static_cast<opcua::ValueRank>(rank));
    }

    void VariableTypeNode::setArrayDimensions(QList<quint32>& dimensions) {
        opcua::Span<const uint32_t> span(dimensions.data(), static_cast<std::size_t>(dimensions.size()));
        handle().writeArrayDimensions(span);
    }

    void VariableTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
