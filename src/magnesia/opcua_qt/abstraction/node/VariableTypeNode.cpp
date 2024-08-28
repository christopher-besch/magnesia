#include "VariableTypeNode.hpp"

#include "../../../qt_version_check.hpp"
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

    std::optional<std::vector<quint32>> VariableTypeNode::getArrayDimensions() {
        try {
            auto                 vector = handle().readArrayDimensions();
            std::vector<quint32> list{vector.begin(), vector.end()};
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

    void VariableTypeNode::setArrayDimensions(std::vector<quint32>& dimensions) {
        handle().writeArrayDimensions(dimensions);
    }

    void VariableTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
