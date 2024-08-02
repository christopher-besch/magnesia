#pragma once

#include <cstdint>

#include <open62541pp/types/Builtin.h>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Used for the BrowseName attribute.
     *
     * @see Node
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.3
     */
    class QualifiedName {
      public:
        explicit QualifiedName(opcua::QualifiedName qualified_name);
        QualifiedName(uint16_t namespace_index, QString& name);

        /**
         * Get the namespace index.
         */
        [[nodiscard]] uint16_t getNameSpaceIndex() const;

        /**
         * Get the name.
         */
        [[nodiscard]] QString getName() const;

        /**
         * Get the underlying qualified name
         */
        [[nodiscard]] const opcua::QualifiedName& handle() const noexcept;

        /**
         * Get the underlying qualified name
         */
        [[nodiscard]] opcua::QualifiedName& handle() noexcept;

      private:
        opcua::QualifiedName m_qualified_name;
    };
} // namespace magnesia::opcua_qt::abstraction
