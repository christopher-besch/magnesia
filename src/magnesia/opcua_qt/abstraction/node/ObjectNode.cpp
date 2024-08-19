#include "ObjectNode.hpp"

#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    ObjectNode::ObjectNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<EventNotifierBitmask> ObjectNode::getEventNotifierType() {
        return EventNotifierBitmask(handle().readEventNotifier());
    }

    void ObjectNode::setEventNotifierType(EventNotifierBitmask type) {
        handle().writeEventNotifier(type.handle());
    }
} // namespace magnesia::opcua_qt::abstraction
