#pragma once

#include "../../../qt_version_check.hpp"
#include "../AccessLevelBitmask.hpp"
#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "Node.hpp"

#include <optional>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

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

        [[nodiscard]] std::optional<DataValue>            getDataValue() override;
        [[nodiscard]] std::optional<NodeId>               getDataType() override;
        [[nodiscard]] std::optional<ValueRank>            getValueRank() override;
        [[nodiscard]] std::optional<std::vector<quint32>> getArrayDimensions() override;
        [[nodiscard]] std::optional<AccessLevelBitmask>   getAccessLevel() override;
        [[nodiscard]] std::optional<AccessLevelBitmask>   getUserAccessLevel() override;
        [[nodiscard]] std::optional<double>               getMinimumSamplingInterval() override;
        [[nodiscard]] std::optional<bool>                 isHistorizing() override;

        void setDataValue(const DataValue& value) override;
        void setDataValue(const Variant& value) override;
        void setDataType(const NodeId& data_type) override;
        void setValueRank(ValueRank rank) override;
        void setArrayDimensions(const std::vector<quint32>& dimensions) override;
        void setAccessLevel(AccessLevelBitmask mask) override;
        void setUserAccessLevel(AccessLevelBitmask mask) override;
        void setHistorizing(bool historizing) override;
    };
} // namespace magnesia::opcua_qt::abstraction
