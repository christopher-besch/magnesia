#include "Endpoint.hpp"

#include "MessageSecurityMode.hpp"

#include <string>
#include <utility>

#include <open62541/types.h>
#include <open62541/types_generated.h>
#include <open62541pp/types/Composed.h>

#include <QString>
#include <QUrl>

namespace magnesia::opcua_qt {
    Endpoint::Endpoint(opcua::EndpointDescription endpoint) : m_endpoint(std::move(endpoint)) {}

    Endpoint::Endpoint(const QUrl& endpoint_url, const QString& endpoint_security_policy_uri,
                       opcua_qt::MessageSecurityMode endpoint_message_security_mode) {
        auto test                     = endpoint_url.toString().toStdString();
        m_endpoint                    = opcua::EndpointDescription();
        m_endpoint->endpointUrl       = UA_String_fromChars(test.c_str());
        m_endpoint->securityMode      = static_cast<UA_MessageSecurityMode>(endpoint_message_security_mode);
        m_endpoint->securityPolicyUri = UA_String_fromChars(endpoint_security_policy_uri.toStdString().c_str());
    }

    QUrl Endpoint::getEndpointUrl() const noexcept {
        return {QString::fromStdString(std::string(m_endpoint.getEndpointUrl()))};
    }

    QString Endpoint::getSecurityPolicyUri() const noexcept {
        return QString::fromStdString(std::string(m_endpoint.getSecurityPolicyUri()));
    }

    MessageSecurityMode Endpoint::getSecurityMode() const noexcept {
        return static_cast<MessageSecurityMode>(m_endpoint.getSecurityMode());
    }

    opcua::EndpointDescription& Endpoint::handle() noexcept {
        return m_endpoint;
    }

    const opcua::EndpointDescription& Endpoint::handle() const noexcept {
        return m_endpoint;
    }
} // namespace magnesia::opcua_qt
