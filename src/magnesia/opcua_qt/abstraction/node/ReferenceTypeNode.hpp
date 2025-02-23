#pragma once

#include "../LocalizedText.hpp"
#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class ReferenceTypeNode
     * @brief Subclass of Node for ReferenceType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.3
     */
    class ReferenceTypeNode : public Node {
      public:
        /**
         * @param node Node
         * @param parent Parent of the ReferenceTypeNode.
         */
        explicit ReferenceTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] const LocalizedText* getInverseName() override;
        [[nodiscard]] std::optional<bool>  isAbstract() override;
        [[nodiscard]] std::optional<bool>  isSymmetric() override;
    };
} // namespace magnesia::opcua_qt::abstraction
