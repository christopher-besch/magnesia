#include "MethodNode.hpp"

#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    MethodNode::MethodNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> MethodNode::isExecutable() {
        return handle().readExecutable();
    }

    std::optional<bool> MethodNode::isUserExecutable() {
        return handle().readUserExecutable();
    }
} // namespace magnesia::opcua_qt::abstraction
