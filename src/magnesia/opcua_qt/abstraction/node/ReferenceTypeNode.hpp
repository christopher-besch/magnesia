#pragma once

#include "Node.hpp"
#include "opcua_qt/abstraction/LocalizedText.hpp"
#include "qt_version_check.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QList>
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

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
        explicit ReferenceTypeNode(opcua::Node<opcua::Client> node);

        [[nodiscard]] std::optional<LocalizedText> getInverseName() override;
        [[nodiscard]] std::optional<bool>          isAbstract() override;
        [[nodiscard]] std::optional<bool>          isSymmetric() override;

        void setAbstract(bool abstract) override;
        void setInverseName(LocalizedText& name) override;
        void setSymmetric(bool symmetric) override;
    };
} // namespace magnesia::opcua_qt::abstraction
