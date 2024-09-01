#include "Node.hpp"

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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

namespace magnesia::opcua_qt::abstraction {
    Node::Node(opcua::Node<opcua::Client> node, QObject* parent) : QObject(parent), m_node(std::move(node)) {}

    NodeId Node::getNodeId() const {
        return NodeId(m_node.id());
    }

    NodeClass Node::getNodeClass() {
        return wrapCache(&Cache::node_class, [this] { return static_cast<NodeClass>(m_node.readNodeClass()); });
    }

    const QualifiedName& Node::getBrowseName() {
        return wrapCache(&Cache::browse_name, [this] { return QualifiedName{m_node.readBrowseName()}; });
    }

    const LocalizedText& Node::getDisplayName() {
        return wrapCache(&Cache::display_name, [this] { return LocalizedText{m_node.readDisplayName()}; });
    }

    const LocalizedText* Node::getDescription() {
        try {
            return &wrapCache(&Cache::description, [this] { return LocalizedText{m_node.readDescription()}; });
        } catch (opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<WriteMaskBitmask> Node::getWriteMask() {
        try {
            return wrapCache(&Cache::write_mask, [this] { return WriteMaskBitmask{m_node.readWriteMask()}; });
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<WriteMaskBitmask> Node::getUserWriteMask() {
        try {
            return wrapCache(&Cache::write_mask, [this] { return WriteMaskBitmask{m_node.readUserWriteMask()}; });
        } catch (opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    Node* Node::getParent() {
        return wrapCache(&Cache::parent, [this] { return fromOPCUANode(m_node.browseParent(), parent()); });
    }

    const std::vector<Node*>& Node::getChildren() {
        return wrapCache(&Cache::children, [this] {
            std::vector<Node*> nodes;

            for (const auto& node : m_node.browseChildren()) {
                if (auto* specific_node = Node::fromOPCUANode(node, parent()); specific_node != nullptr) {
                    specific_node->m_cache.parent = this;
                    nodes.push_back(specific_node);
                }
            }

            return nodes;
        });
    }

    const std::vector<ReferenceDescription>& Node::getReferences() {
        return wrapCache(&Cache::references, [this]() -> std::vector<ReferenceDescription> {
            auto references = m_node.browseReferences();
            return {references.begin(), references.end()};
        });
    }

    const LocalizedText* Node::getInverseName() {
        return nullptr;
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

    const DataValue* Node::getDataValue() {
        return nullptr;
    }

    std::optional<NodeId> Node::getDataType() {
        return std::nullopt;
    }

    std::optional<ValueRank> Node::getValueRank() {
        return std::nullopt;
    }

    const std::vector<std::uint32_t>* Node::getArrayDimensions() {
        return nullptr;
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
        invalidateCache(&Cache::display_name);
    }

    void Node::setDescription(LocalizedText& description) {
        m_node.writeDescription(description.handle());
        invalidateCache(&Cache::description);
    }

    void Node::setWriteMask(WriteMaskBitmask mask) {
        m_node.writeWriteMask(mask.handle());
        invalidateCache(&Cache::write_mask);
    }

    void Node::setUserWriteMask(WriteMaskBitmask mask) {
        m_node.writeUserWriteMask(mask.handle());
        invalidateCache(&Cache::user_write_mask);
    }

    void Node::setInverseName(const LocalizedText& /*name*/) {}

    void Node::setAbstract(bool /*abstract*/) {}

    void Node::setSymmetric(bool /*symmetric*/) {}

    void Node::setEventNotifierType(EventNotifierBitmask /*type*/) {}

    void Node::setDataValue(const DataValue& /*value*/) {}

    void Node::setDataType(const NodeId& /*data_type*/) {}

    void Node::setDataValue(const Variant& /*value*/) {}

    void Node::setValueRank(ValueRank /*rank*/) {}

    void Node::setArrayDimensions(const std::vector<std::uint32_t>& /*dimensions*/) {}

    void Node::setAccessLevel(AccessLevelBitmask /*mask*/) {}

    void Node::setUserAccessLevel(AccessLevelBitmask /*mask*/) {}

    void Node::setHistorizing(bool /*historizing*/) {}

    std::vector<Variant> Node::callMethod(const NodeId& /*method_id*/, const std::vector<Variant>& /*args*/) {
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

    std::optional<std::size_t> Node::childrenCountCached() const {
        if (m_cache.children.has_value()) {
            return m_cache.children->size();
        }
        return std::nullopt;
    }

    bool Node::operator==(const Node& other) const {
        return m_node == other.m_node;
    }

    const std::optional<LocalizedText>& Node::setCacheDisplayName(std::optional<LocalizedText> display_name) {
        return m_cache.display_name = std::move(display_name);
    }

    const std::optional<DataValue>& Node::setCacheDataValue(std::optional<DataValue> data_value) {
        return m_cache.data_value = std::move(data_value);
    }
} // namespace magnesia::opcua_qt::abstraction
