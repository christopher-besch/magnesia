#pragma once

#include "../LocalizedText.hpp"
#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Subclass of Node for ReferenceType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.3
     */
    class ReferenceTypeNode : public Node {
      public:
        explicit ReferenceTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<LocalizedText> getInverseName() override;
        [[nodiscard]] std::optional<bool>          isAbstract() override;
        [[nodiscard]] std::optional<bool>          isSymmetric() override;

        void setAbstract(bool abstract) override;
        void setInverseName(LocalizedText& name) override;
        void setSymmetric(bool symmetric) override;
    };
} // namespace magnesia::opcua_qt::abstraction
