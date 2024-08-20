#pragma once

#include <cstdint>

#include <open62541pp/types/NodeId.h>

#include <QString>
#include <QVariant>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Used to uniquely identify a node within the global namespace.
     *
     * @see Node
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.2.2
     */
    class NodeId {
      public:
        explicit NodeId(opcua::NodeId node_id);
        /**
         * Get the namespace index.
         */
        [[nodiscard]] uint16_t getNamespaceIndex() const noexcept;

        /**
         * Get the Identifier.
         */
        [[nodiscard]] QVariant getIdentifier() const;

        /**
         * Get a string representation of this node id.
         */
        [[nodiscard]] QString toString() const;

        /**
         * Get the underlying node id.
         */
        [[nodiscard]] const opcua::NodeId& handle() const noexcept;

        /**
         * Get the underlying node id.
         */
        [[nodiscard]] opcua::NodeId& handle() noexcept;

        bool operator==(const NodeId& other) const;

      private:
        opcua::NodeId m_node_id;
    };
} // namespace magnesia::opcua_qt::abstraction
