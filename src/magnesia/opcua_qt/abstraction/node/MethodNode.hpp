#pragma once

#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class MethodNode
     * @brief Subclass of Node for Method NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.7
     */
    class MethodNode : public Node {
      public:
        /**
         * @param node Node
         * @param parent Parent of the MethodNode.
         */
        explicit MethodNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<bool> isExecutable() override;
        [[nodiscard]] std::optional<bool> isUserExecutable() override;
    };
} // namespace magnesia::opcua_qt::abstraction
