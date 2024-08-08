#pragma once

#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

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
        explicit ObjectNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<EventNotifierBitmask> getEventNotifierType() override;

        void setEventNotifierType(EventNotifierBitmask type) override;
    };
} // namespace magnesia::opcua_qt::abstraction
