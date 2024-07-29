#pragma once

#include "LocalizedText.hpp"
#include "NodeId.hpp"
#include "QualifiedName.hpp"

#include <open62541pp/types/Composed.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Description of a reference of a Node.
     *
     * @see Node
     *
     * See https://reference.opcfoundation.org/Core/Part4/v105/docs/7.30
     */
    class ReferenceDescription {
      public:
        explicit ReferenceDescription(opcua::ReferenceDescription reference_description);

        /**
         * Get the node id for the node that describes this reference type.
         */
        [[nodiscard]] NodeId getReferenceType() const noexcept;

        /**
         * Get the browse name of this reference type. This is a shorthand for the BrowseName attribute of the node with
         * the id given by getReferenceType.
         */
        [[nodiscard]] QualifiedName getBrowseName() const noexcept;

        /**
         * Get the display name of this reference type. This is a shorthand for the DisplayName attribute of the node
         * with the id given by getReferenceType.
         */
        [[nodiscard]] LocalizedText getDisplayName() const noexcept;

        /**
         * Get the underlying reference description
         */
        [[nodiscard]] const opcua::ReferenceDescription& handle() const noexcept;

        /**
         * Get the underlying reference description
         */
        [[nodiscard]] opcua::ReferenceDescription& handle() noexcept;

      private:
        opcua::ReferenceDescription m_description;
    };
} // namespace magnesia::opcua_qt::abstraction
