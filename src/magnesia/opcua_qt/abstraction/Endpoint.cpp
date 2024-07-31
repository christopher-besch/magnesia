#include "Endpoint.hpp"

#include "MessageSecurityMode.hpp"

#include <string>
#include <utility>

#include <open62541pp/types/Composed.h>

#include <QString>
#include <QUrl>

namespace magnesia::opcua_qt {
    Endpoint::Endpoint(opcua::EndpointDescription endpoint) : m_endpoint(std::move(endpoint)) {}

    QUrl Endpoint::getEndpointUrl() const noexcept {
        return {QString::fromStdString(std::string(m_endpoint.getEndpointUrl()))};
    }

    QString Endpoint::getSecurityPolicyUri() noexcept {
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
