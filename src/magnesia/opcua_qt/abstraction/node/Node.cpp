#include "Node.hpp"

#include "../../../qt_version_check.hpp"
#include "../AccessLevelBitmask.hpp"
#include "../DataValue.hpp"
#include "../EventNotifierBitmask.hpp"
#include "../LocalizedText.hpp"
#include "../NodeClass.hpp"
#include "../NodeId.hpp"
#include "../QualifiedName.hpp"
#include "../ReferenceDescription.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "../WriteMaskBitmask.hpp"
#include "DataTypeNode.hpp"
#include "MethodNode.hpp"
#include "ObjectNode.hpp"
#include "ObjectTypeNode.hpp"
#include "ReferenceTypeNode.hpp"
#include "VariableNode.hpp"
#include "VariableTypeNode.hpp"
#include "ViewNode.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>

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
        if (m_cache_display_name.has_value()) {
            return m_cache_display_name.value();
        }

        m_cache_display_name = LocalizedText(m_node.readDisplayName());
        return m_cache_display_name.value();
    }

    std::optional<LocalizedText> Node::getDescription() {
        try {
            return LocalizedText(m_node.readDescription());
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<WriteMaskBitmask> Node::getWriteMask() {
        try {
            return WriteMaskBitmask(m_node.readWriteMask());
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<WriteMaskBitmask> Node::getUserWriteMask() {
        try {
            return WriteMaskBitmask(m_node.readUserWriteMask());
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    Node* Node::getParent() {
        if (m_cache_parent.has_value()) {
            return m_cache_parent.value();
        }

        m_cache_parent = fromOPCUANode(m_node.browseParent(), parent());
        return m_cache_parent.value();
    }

    QList<Node*> Node::getChildren() {
        if (m_cache_children.has_value()) {
            return m_cache_children.value();
        }

        QList<Node*> nodes{};

        for (const auto& node : m_node.browseChildren()) {
            if (auto* specific_node = Node::fromOPCUANode(node, parent()); specific_node != nullptr) {
                specific_node->m_cache_parent = this;
                nodes.append(specific_node);
            }
        }

        m_cache_children = nodes;
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

    bool Node::operator==(const Node& other) const {
        return m_node == other.m_node;
    }

    const std::optional<LocalizedText>& Node::setCacheDisplayName(std::optional<LocalizedText> display_name) {
        return m_cache_display_name = std::move(display_name);
    }

    const std::optional<DataValue>& Node::setCacheDataValue(std::optional<DataValue> data_value) {
        return m_cache_data_value = std::move(data_value);
    }

    const std::optional<DataValue>& Node::getCacheDataValue() {
        return m_cache_data_value;
    }
} // namespace magnesia::opcua_qt::abstraction
