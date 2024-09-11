#include "ReferenceTypeNode.hpp"

#include "../LocalizedText.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    ReferenceTypeNode::ReferenceTypeNode(opcua::Node<opcua::Client> node, QObject* parent)
        : Node(std::move(node), parent) {}

    const LocalizedText* ReferenceTypeNode::getInverseName() {
        try {
            return &wrapCache(&Cache::inverse_name, [this] { return LocalizedText{handle().readInverseName()}; });
        } catch (opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<bool> ReferenceTypeNode::isAbstract() {
        return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
    }

    std::optional<bool> ReferenceTypeNode::isSymmetric() {
        return wrapCache(&Cache::is_symmetric, [this] { return handle().readSymmetric(); });
    }
} // namespace magnesia::opcua_qt::abstraction
