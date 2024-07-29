#include "ReferenceTypeNode.hpp"

#include "Node.hpp"
#include "opcua_qt/abstraction/LocalizedText.hpp"
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
    ReferenceTypeNode::ReferenceTypeNode(opcua::Node<opcua::Client> node, QObject* parent)
        : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::ReferenceType);
    }

    std::optional<LocalizedText> ReferenceTypeNode::getInverseName() {
        return LocalizedText(handle().readInverseName());
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
