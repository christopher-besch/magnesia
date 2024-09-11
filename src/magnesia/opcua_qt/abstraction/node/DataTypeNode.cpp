#include "DataTypeNode.hpp"

#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    DataTypeNode::DataTypeNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    std::optional<bool> DataTypeNode::isAbstract() {
        return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
    }
} // namespace magnesia::opcua_qt::abstraction
