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

    std::optional<LocalizedText> ReferenceTypeNode::getInverseName() {
        try {
            return LocalizedText(handle().readInverseName());
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> ReferenceTypeNode::isAbstract() {
        return handle().readIsAbstract();
    }

    std::optional<bool> ReferenceTypeNode::isSymmetric() {
        return handle().readSymmetric();
    }

    void ReferenceTypeNode::setInverseName(LocalizedText& name) {
        handle().writeInverseName(name.handle());
    }

    void ReferenceTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }

    void ReferenceTypeNode::setSymmetric(bool symmetric) {
        handle().writeSymmetric(symmetric);
    }
} // namespace magnesia::opcua_qt::abstraction
