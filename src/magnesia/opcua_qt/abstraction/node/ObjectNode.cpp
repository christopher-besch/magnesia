#include "ObjectNode.hpp"

#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    ObjectNode::ObjectNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<EventNotifierBitmask> ObjectNode::getEventNotifierType() {
        try {
            return wrapCache(&Cache::event_notifier,
                             [this] { return EventNotifierBitmask{handle().readEventNotifier()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
