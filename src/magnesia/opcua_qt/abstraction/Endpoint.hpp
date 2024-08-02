#pragma once

#include "MessageSecurityMode.hpp"

#include <open62541pp/types/Composed.h>

#include <QString>
#include <QUrl>

namespace magnesia::opcua_qt {
    /**
     * @class Endpoint
     * @brief Wraps a OPC UA EndpointDescription
     */
    class Endpoint {
      public:
        explicit Endpoint(opcua::EndpointDescription endpoint);
        /**
         * @brief Gets the endpoint url
         *
         * @return Returns a QUrl
         */
        [[nodiscard]] QUrl getEndpointUrl() const noexcept;
        /**
         * @brief Gets the security policy uri
         *
         * @return Returns a QString
         */
        [[nodiscard]] QString getSecurityPolicyUri() const noexcept;
        /**
         * @brief Gets the security mode
         *
         * @return Returns a MessageSecurityMode
         */
        [[nodiscard]] MessageSecurityMode getSecurityMode() const noexcept;
        /**
         * @brief Returns the underlying opcua::EndpointDescription
         */
        [[nodiscard]] opcua::EndpointDescription& handle() noexcept;
        /**
         * @brief Returns the underlying opcua::EndpointDescription
         */
        [[nodiscard]] const opcua::EndpointDescription& handle() const noexcept;

      private:
        opcua::EndpointDescription m_endpoint;
    };
} // namespace magnesia::opcua_qt
