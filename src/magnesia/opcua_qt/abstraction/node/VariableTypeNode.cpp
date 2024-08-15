#include "VariableTypeNode.hpp"

#include "../../../qt_version_check.hpp"
#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "Node.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
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
    VariableTypeNode::VariableTypeNode(opcua::Node<opcua::Client> node, QObject* parent)
        : Node(std::move(node), parent) {}

    std::optional<DataValue> VariableTypeNode::getDataValue() {
        try {
            return DataValue(handle().readDataValue());
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

    std::optional<QList<quint32>> VariableTypeNode::getArrayDimensions() {
        try {
            auto vector = handle().readArrayDimensions();
            auto list   = QList<quint32>();

            list.reserve(static_cast<qsizetype>(vector.size()));
            std::copy(vector.begin(), vector.end(), std::back_inserter(list));

            return list;
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
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
