#include "MethodNode.hpp"

#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    MethodNode::MethodNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> MethodNode::isExecutable() {
        try {
            return wrapCache(&Cache::is_executable, [this] { return handle().readExecutable(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> MethodNode::isUserExecutable() {
        try {
            return wrapCache(&Cache::is_user_executable, [this] { return handle().readUserExecutable(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
