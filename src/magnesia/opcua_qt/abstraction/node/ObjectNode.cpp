#include "ObjectNode.hpp"

#include "Node.hpp"
#include "opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "qt_version_check.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    ObjectNode::ObjectNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::Object);
    }

    std::optional<EventNotifierBitmask> ObjectNode::getEventNotifierType() {
        return EventNotifierBitmask(handle().readEventNotifier());
    }

    void ObjectNode::setEventNotifierType(EventNotifierBitmask type) {
        handle().writeEventNotifier(type.handle());
    }
} // namespace magnesia::opcua_qt::abstraction
