#pragma once

#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class ViewNode
     * @brief Subclass of Node for View NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.4
     */
    class ViewNode : public Node {
      public:
        /**
         * @param node Node
         * @param parent Parent of the ViewNode.
         */
        explicit ViewNode(opcua::Node<opcua::Client> node, QObject* parent);

        /**
         * Retrieves weather the ViewNode contains loops or not.
         * @return true, if node contains loops, false if not. nullopt if Node is not a ViewNode.
         */
        [[nodiscard]] std::optional<bool> containsNoLoops() override;

        /**
         * Retrieves the EventNotifierBitmask
         * @return EventNotifierBitmask of the node if it exists, nullopt otherwise.
         */
        [[nodiscard]] std::optional<EventNotifierBitmask> getEventNotifierType() override;

        /**
         * Sets the EventNotifierType of the node.
         * @param type EventNotifierType
         */
        void setEventNotifierType(EventNotifierBitmask type) override;
    };
} // namespace magnesia::opcua_qt::abstraction
