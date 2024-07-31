#include "ObjectTypeNode.hpp"

#include "Node.hpp"
#include "qt_version_check.hpp"

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
    ObjectTypeNode::ObjectTypeNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::ObjectType);
    }

    std::optional<bool> ObjectTypeNode::isAbstract() {
        return handle().readIsAbstract();
    }

    void ObjectTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
