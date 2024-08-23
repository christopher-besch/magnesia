#pragma once

#include "../../../qt_version_check.hpp"
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
     * Subclass of Node for VariableType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.6.5
     */
    class VariableTypeNode : public Node {
      public:
        explicit VariableTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<DataValue>            getDataValue() override;
        [[nodiscard]] std::optional<NodeId>               getDataType() override;
        [[nodiscard]] std::optional<ValueRank>            getValueRank() override;
        [[nodiscard]] std::optional<std::vector<quint32>> getArrayDimensions() override;
        [[nodiscard]] std::optional<bool>                 isAbstract() override;

        void setDataValue(DataValue& value) override;
        void setDataValue(Variant& value) override;
        void setDataType(NodeId& data_type) override;
        void setValueRank(ValueRank rank) override;
        void setArrayDimensions(std::vector<quint32>& dimensions) override;
        void setAbstract(bool abstract) override;
    };
} // namespace magnesia::opcua_qt::abstraction
