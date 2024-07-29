#include "Node.hpp"

#include "DataTypeNode.hpp"
#include "MethodNode.hpp"
#include "ObjectNode.hpp"
#include "ObjectTypeNode.hpp"
#include "ReferenceTypeNode.hpp"
#include "VariableNode.hpp"
#include "VariableTypeNode.hpp"
#include "ViewNode.hpp"
#include "opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "opcua_qt/abstraction/DataValue.hpp"
#include "opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "opcua_qt/abstraction/LocalizedText.hpp"
#include "opcua_qt/abstraction/NodeClass.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "opcua_qt/abstraction/QualifiedName.hpp"
#include "opcua_qt/abstraction/ReferenceDescription.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"
#include "opcua_qt/abstraction/Variant.hpp"
#include "opcua_qt/abstraction/WriteMaskBitmask.hpp"
#include "qt_version_check.hpp"

#include <cassert>
#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>
#include <QSharedPointer>
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    Node::Node(opcua::Node<opcua::Client> node, QObject* parent) : QObject(parent), m_node(std::move(node)) {}

    NodeId Node::getNodeId() const {
        return NodeId(m_node.id());
    }

    NodeClass Node::getNodeClass() {
        return static_cast<NodeClass>(m_node.readNodeClass());
    }

    QualifiedName Node::getBrowseName() {
        return QualifiedName(m_node.readBrowseName());
    }

    LocalizedText Node::getDisplayName() {
        return LocalizedText(m_node.readDisplayName());
    }

    LocalizedText Node::getDescription() {
        return LocalizedText(m_node.readDescription());
    }

    std::optional<WriteMaskBitmask> Node::getWriteMask() {
        return WriteMaskBitmask(m_node.readWriteMask());
    }

    std::optional<WriteMaskBitmask> Node::getUserWriteMask() {
        return WriteMaskBitmask(m_node.readUserWriteMask());
    }

    Node* Node::getParent() {
        return fromOPCUANode(m_node.browseParent(), parent());
    }

    QList<Node*> Node::getChildren() {
        QList<Node*> nodes{};

        for (const auto& node : m_node.browseChildren()) {
            if (auto* specific_node = Node::fromOPCUANode(node, parent()); specific_node != nullptr) {
                nodes.append(specific_node);
            }
        }
        return nodes;
    }

    QList<ReferenceDescription> Node::getReferences() {
        QList<ReferenceDescription> references = QList<ReferenceDescription>();

        for (const auto& reference : m_node.browseReferences()) {
            references.append(ReferenceDescription(reference));
        }

        return references;
    }

    std::optional<LocalizedText> Node::getInverseName() {
        return std::nullopt;
    }

    std::optional<bool> Node::isAbstract() {
        return std::nullopt;
    }

    std::optional<bool> Node::isSymmetric() {
        return std::nullopt;
    }

    std::optional<bool> Node::containsNoLoops() {
        return std::nullopt;
    }

    std::optional<EventNotifierBitmask> Node::getEventNotifierType() {
        return std::nullopt;
    }

    std::optional<DataValue> Node::getDataValue() {
        return std::nullopt;
    }

    std::optional<NodeId> Node::getDataType() {
        return std::nullopt;
    }

    std::optional<ValueRank> Node::getValueRank() {
        return std::nullopt;
    }

    std::optional<QList<quint32>> Node::getArrayDimensions() {
        return std::nullopt;
    }

    std::optional<AccessLevelBitmask> Node::getAccessLevel() {
        return std::nullopt;
    }

    std::optional<AccessLevelBitmask> Node::getUserAccessLevel() {
        return std::nullopt;
    }

    std::optional<double> Node::getMinimumSamplingInterval() {
        return std::nullopt;
    }

    std::optional<bool> Node::isHistorizing() {
        return std::nullopt;
    }

    std::optional<bool> Node::isExecutable() {
        return std::nullopt;
    }

    std::optional<bool> Node::isUserExecutable() {
        return std::nullopt;
    }

    void Node::setDisplayName(LocalizedText& name) {
        m_node.writeDisplayName(name.handle());
    }

    void Node::setDescription(LocalizedText& description) {
        m_node.writeDescription(description.handle());
    }

    void Node::setWriteMask(WriteMaskBitmask mask) {
        m_node.writeWriteMask(mask.handle());
    }

    void Node::setUserWriteMask(WriteMaskBitmask mask) {
        m_node.writeUserWriteMask(mask.handle());
    }

    void Node::setInverseName(LocalizedText& /*name*/) {}

    void Node::setAbstract(bool /*abstract*/) {}

    void Node::setSymmetric(bool /*symmetric*/) {}

    void Node::setEventNotifierType(EventNotifierBitmask /*type*/) {}

    void Node::setDataValue(DataValue& /*value*/) {}

    void Node::setDataType(NodeId& /*data_type*/) {}

    void Node::setDataValue(Variant& /*value*/) {}

    void Node::setValueRank(ValueRank /*rank*/) {}

    void Node::setArrayDimensions(QList<quint32>& /*dimensions*/) {}

    void Node::setAccessLevel(AccessLevelBitmask /*mask*/) {}

    void Node::setUserAccessLevel(AccessLevelBitmask /*mask*/) {}

    void Node::setHistorizing(bool /*historizing*/) {}

    QList<Variant> Node::callMethod(NodeId& /*method_id*/, QList<Variant>& /*args*/) {
        return {};
    }

    const opcua::Node<opcua::Client>& Node::handle() const noexcept {
        return m_node;
    }

    opcua::Node<opcua::Client>& Node::handle() noexcept {
        return m_node;
    }

    Node* Node::fromOPCUANode(opcua::Node<opcua::Client> node, QObject* parent) {
        switch (node.readNodeClass()) {
            case opcua::NodeClass::DataType:
                return new DataTypeNode(node, parent);
            case opcua::NodeClass::ReferenceType:
                return new ReferenceTypeNode(node, parent);
            case opcua::NodeClass::ObjectType:
                return new ObjectTypeNode(node, parent);
            case opcua::NodeClass::VariableType:
                return new VariableTypeNode(node, parent);
            case opcua::NodeClass::Variable:
                return new VariableNode(node, parent);
            case opcua::NodeClass::Object:
                return new ObjectNode(node, parent);
            case opcua::NodeClass::Method:
                return new MethodNode(node, parent);
            case opcua::NodeClass::View:
                return new ViewNode(node, parent);
                // takes care of opcua::NodeClass::Unspecified
            default:
                return nullptr;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
