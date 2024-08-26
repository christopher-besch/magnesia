#pragma once

#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class DataTypeNode
     * @brief Subclass of Node for DataType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.8.3
     */
    class DataTypeNode : public Node {
      public:
        /**
         * @param node Node
         * @param parent Parent of the DataTypeNode.
         */
        explicit DataTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<bool> isAbstract() override;

        void setAbstract(bool abstract) override;
    };
} // namespace magnesia::opcua_qt::abstraction
