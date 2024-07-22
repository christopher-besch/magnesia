#pragma once

#include "Node.hpp"
#include "opcua_qt/abstraction/EventNotifierBitmask.hpp"
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
     * Subclass of Node for Object NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.5.1
     */
    class ObjectNode : public Node {
      public:
        explicit ObjectNode(opcua::Node<opcua::Client> node);

        [[nodiscard]] std::optional<EventNotifierBitmask> getEventNotifierType() override;

        void setEventNotifierType(EventNotifierBitmask type) override;
    };
} // namespace magnesia::opcua_qt::abstraction
