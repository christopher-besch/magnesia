#include "DataTypeNode.hpp"

#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    DataTypeNode::DataTypeNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> DataTypeNode::isAbstract() {
        try {
            return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
