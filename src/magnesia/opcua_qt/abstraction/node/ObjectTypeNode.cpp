#include "ObjectTypeNode.hpp"

#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    ObjectTypeNode::ObjectTypeNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> ObjectTypeNode::isAbstract() {
        return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
    }

    void ObjectTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
        invalidateCache(&Cache::is_abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
