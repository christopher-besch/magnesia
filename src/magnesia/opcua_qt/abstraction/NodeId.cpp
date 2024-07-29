#include "NodeId.hpp"

#include <cstdint>
#include <utility>

#include <open62541pp/types/NodeId.h>

namespace magnesia::opcua_qt::abstraction {
    NodeId::NodeId(opcua::NodeId node_id) : m_node_id(std::move(node_id)) {}

    uint16_t NodeId::getNamespaceIndex() const noexcept {
        return m_node_id.getNamespaceIndex();
    }

    QString NodeId::toString() const {
        return QString::fromStdString(m_node_id.toString());
    }

    const opcua::NodeId& NodeId::handle() const noexcept {
        return m_node_id;
    }

    opcua::NodeId& NodeId::handle() noexcept {
        return m_node_id;
    }
} // namespace magnesia::opcua_qt::abstraction
