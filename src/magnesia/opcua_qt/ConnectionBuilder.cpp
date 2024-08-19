#include "ConnectionBuilder.hpp"

#include "../qt_version_check.hpp"
#include "ApplicationCertificate.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "abstraction/Endpoint.hpp"

#include <algorithm>
#include <iterator>
#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Result.h>

#include <QList>
#include <QMutexLocker>
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
        const QMutexLocker locker(&m_get_endpoint_mutex);

        m_url = url;
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
        QThreadPool::globalInstance()->start([&] { Q_EMIT endpointsFound(findEndopintsSynchronously()); });
    }

    Connection* ConnectionBuilder::build() {
        if (!m_endpoint.has_value() || m_logger == nullptr) {
            return nullptr;
        }

        std::optional<opcua::Login> login;
        if (m_username.has_value() && m_password.has_value()) {
            login           = opcua::Login();
            login->username = m_username.value().toStdString();
            login->password = m_password.value().toStdString();
        }

        return new Connection(m_endpoint.value(), login, m_certificate, m_trust_list, m_revoked_list, m_logger);
    }

    opcua::Result<QList<Endpoint>> ConnectionBuilder::findEndopintsSynchronously() {
        const QMutexLocker locker(&m_get_endpoint_mutex);

        Q_ASSERT(m_url.has_value());
        try {
            const auto endpoint_descriptions = opcua::Client{}.getEndpoints(m_url.value().toString().toStdString());
            QList<Endpoint> endpoints;
            std::ranges::transform(endpoint_descriptions, std::back_inserter(endpoints),
                                   [](auto endpoint) { return Endpoint{std::move(endpoint)}; });
            return endpoints;
        } catch (const opcua::BadStatus& status) {
            return opcua::BadResult(status.code());
        }
    }

    const std::optional<QUrl>& ConnectionBuilder::getUrl() const {
        return m_url;
    }

    const std::optional<Endpoint>& ConnectionBuilder::getEndpoint() const {
        return m_endpoint;
    }

    Logger* ConnectionBuilder::getLogger() const {
        return m_logger;
    }

    const std::optional<QString>& ConnectionBuilder::getUsername() const {
        return m_username;
    }

    const std::optional<QString>& ConnectionBuilder::getPassword() const {
        return m_password;
    }

    const std::optional<ApplicationCertificate>& ConnectionBuilder::getCertificate() const {
        return m_certificate;
    }

    const QList<QSslCertificate>& ConnectionBuilder::getTrustList() const {
        return m_trust_list;
    }

    const QList<QSslCertificate>& ConnectionBuilder::getRevokedList() const {
        return m_revoked_list;
    }
} // namespace magnesia::opcua_qt
