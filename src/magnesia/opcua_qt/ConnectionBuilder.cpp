#include "ConnectionBuilder.hpp"

#include "Application.hpp"
#include "ApplicationCertificate.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "abstraction/Endpoint.hpp"
#include "qt_version_check.hpp"

#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Logger.h>
#include <open62541pp/types/Composed.h>

#include <QList>
#include <QSslCertificate>
#include <QString>
#include <QThreadPool>
#include <QUrl>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt {
    ConnectionBuilder& ConnectionBuilder::url(const QUrl& url) noexcept {
        m_get_endpoint_mutex.lock();
        m_url = url;
        m_get_endpoint_mutex.unlock();
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::endpoint(const Endpoint& endpoint) noexcept {
        // we can't check that the endpoit exists because the find endpoint function may not have been called yet.
        m_endpoint = endpoint;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::logger(Logger* logger) noexcept {
        m_logger = logger;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::username(const QString& username) noexcept {
        m_username = username;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::password(const QString& password) noexcept {
        m_password = password;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::certificate(const ApplicationCertificate& certificate) noexcept {
        m_certificate = certificate;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::trustList(const QList<QSslCertificate>& trust_list) noexcept {
        m_trust_list = trust_list;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::revokedList(const QList<QSslCertificate>& revoked_list) noexcept {
        m_revoked_list = revoked_list;
        return *this;
    }

    void ConnectionBuilder::findEndpoints() {
        QThreadPool::globalInstance()->start([&] { findEndopintsSynchronously(); });
    }

    std::optional<QString>&& ConnectionBuilder::getUsername() noexcept {
        return std::move(m_username);
    }

    std::optional<QString>&& ConnectionBuilder::getPassword() noexcept {
        return std::move(m_password);
    }

    std::optional<ApplicationCertificate>&& ConnectionBuilder::getCertificate() noexcept {
        return std::move(m_certificate);
    }

    QList<QSslCertificate>&& ConnectionBuilder::getTrustList() noexcept {
        return std::move(m_trust_list);
    }

    QList<QSslCertificate>&& ConnectionBuilder::getRevokedList() noexcept {
        return std::move(m_revoked_list);
    }

    std::optional<Endpoint>&& ConnectionBuilder::getEndpoint() noexcept {
        return std::move(m_endpoint);
    }

    Logger* ConnectionBuilder::getLogger() noexcept {
        return std::exchange(m_logger, nullptr);
    }

    Connection* ConnectionBuilder::build() {
        if (!m_endpoint.has_value() || m_logger == nullptr) {
            return nullptr;
        }
        return Application::instance().getConnectionManager().createConnection(*this);
    }

    void ConnectionBuilder::findEndopintsSynchronously() {
        // never unlock this as the function may not be called more than once
        m_get_endpoint_mutex.lock();
        Q_ASSERT(m_url.has_value());
        auto                                          client = opcua::Client();
        const std::vector<opcua::EndpointDescription> endpoints =
            client.getEndpoints(m_url.value().toString().toStdString());
        for (const opcua::EndpointDescription& endpoint : endpoints) {
            // store these in the builder to not have them be deleted at the end of the scope
            m_endpoints.append(Endpoint(endpoint));
        }
        Q_EMIT endpointsFound(m_endpoints);
    }
} // namespace magnesia::opcua_qt
