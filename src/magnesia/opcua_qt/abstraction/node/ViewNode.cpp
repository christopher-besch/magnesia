#include "ViewNode.hpp"

#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    ViewNode::ViewNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> ViewNode::containsNoLoops() {
        return wrapCache(&Cache::contains_no_loops, [this] { return handle().readContainsNoLoops(); });
    }

    std::optional<EventNotifierBitmask> ViewNode::getEventNotifierType() {
        return wrapCache(&Cache::event_notifier, [this] { return EventNotifierBitmask{handle().readEventNotifier()}; });
    }
} // namespace magnesia::opcua_qt::abstraction
