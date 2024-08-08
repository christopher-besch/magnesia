#include "ViewNode.hpp"

#include "../../../qt_version_check.hpp"
#include "../EventNotifierBitmask.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Node.h>

#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    ViewNode::ViewNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::View);
    }

    std::optional<bool> ViewNode::containsNoLoops() {
        return handle().readContainsNoLoops();
    }

    std::optional<EventNotifierBitmask> ViewNode::getEventNotifierType() {
        return EventNotifierBitmask(handle().readEventNotifier());
    }

    void ViewNode::setEventNotifierType(EventNotifierBitmask type) {
        handle().writeEventNotifier(type.handle());
    }
} // namespace magnesia::opcua_qt::abstraction
