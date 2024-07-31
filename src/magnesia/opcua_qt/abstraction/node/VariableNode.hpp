#pragma once

#include "Node.hpp"
#include "opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "opcua_qt/abstraction/DataValue.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"
#include "opcua_qt/abstraction/Variant.hpp"
#include "qt_version_check.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    /**
     * Subclass of Node for Variable NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.6.2
     */
    class VariableNode : public Node {
      public:
        explicit VariableNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<DataValue>          getDataValue() override;
        [[nodiscard]] std::optional<NodeId>             getDataType() override;
        [[nodiscard]] std::optional<ValueRank>          getValueRank() override;
        [[nodiscard]] std::optional<QList<quint32>>     getArrayDimensions() override;
        [[nodiscard]] std::optional<AccessLevelBitmask> getAccessLevel() override;
        [[nodiscard]] std::optional<AccessLevelBitmask> getUserAccessLevel() override;
        [[nodiscard]] std::optional<double>             getMinimumSamplingInterval() override;
        [[nodiscard]] std::optional<bool>               isHistorizing() override;

        void setDataValue(DataValue& value) override;
        void setDataValue(Variant& value) override;
        void setDataType(NodeId& data_type) override;
        void setValueRank(ValueRank rank) override;
        void setArrayDimensions(QList<quint32>& dimensions) override;
        void setAccessLevel(AccessLevelBitmask mask) override;
        void setUserAccessLevel(AccessLevelBitmask mask) override;
        void setHistorizing(bool historizing) override;
    };
} // namespace magnesia::opcua_qt::abstraction
