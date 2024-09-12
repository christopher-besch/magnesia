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

    std::optional<NodeClass> Node::getNodeClass() {
        try {
            return wrapCache(&Cache::node_class, [this] { return static_cast<NodeClass>(m_node.readNodeClass()); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    const QualifiedName* Node::getBrowseName() {
        try {
            return &wrapCache(&Cache::browse_name, [this] { return QualifiedName{m_node.readBrowseName()}; });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    const LocalizedText* Node::getDisplayName() {
        try {
            return &wrapCache(&Cache::display_name, [this] { return LocalizedText{m_node.readDisplayName()}; });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    const LocalizedText* Node::getDescription() {
        try {
            return &wrapCache(&Cache::description, [this] { return LocalizedText{m_node.readDescription()}; });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<WriteMaskBitmask> Node::getWriteMask() {
        try {
            return wrapCache(&Cache::write_mask, [this] { return WriteMaskBitmask{m_node.readWriteMask()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<WriteMaskBitmask> Node::getUserWriteMask() {
        try {
            return wrapCache(&Cache::write_mask, [this] { return WriteMaskBitmask{m_node.readUserWriteMask()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    Node* Node::getParent() {
        try {
            return wrapCache(&Cache::parent, [this] { return fromOPCUANode(m_node.browseParent(), parent()); });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    const std::vector<Node*>* Node::getChildren() {
        try {
            return &wrapCache(&Cache::children, [this] {
                std::vector<Node*> nodes;

                for (const auto& node : m_node.browseChildren()) {
                    if (auto* specific_node = Node::fromOPCUANode(node, parent()); specific_node != nullptr) {
                        specific_node->m_cache.parent = this;
                        nodes.push_back(specific_node);
                    }
                }
                return nodes;
            });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    const std::vector<ReferenceDescription>* Node::getReferences() {
        try {
            return &wrapCache(&Cache::references, [this]() -> std::vector<ReferenceDescription> {
                auto references = m_node.browseReferences();
                return {references.begin(), references.end()};
            });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
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
} // namespace magnesia::opcua_qt::abstraction
