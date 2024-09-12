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
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<bool> ReferenceTypeNode::isAbstract() {
        try {
            return wrapCache(&Cache::is_abstract, [this] { return handle().readIsAbstract(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> ReferenceTypeNode::isSymmetric() {
        try {
            return wrapCache(&Cache::is_symmetric, [this] { return handle().readSymmetric(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
