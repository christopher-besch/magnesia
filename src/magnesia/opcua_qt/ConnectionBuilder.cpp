#include "ConnectionBuilder.hpp"

#include "../Application.hpp"
#include "../database_types.hpp"
#include "../qt_version_check.hpp"
#include "ApplicationCertificate.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "abstraction/Endpoint.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Result.h>

#include <QList>
#include <QLoggingCategory>
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

namespace {
    Q_LOGGING_CATEGORY(lc_builder, "magnesia.opcua.builder")
} // namespace

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

    ConnectionBuilder& ConnectionBuilder::certificate(const StorageId& certificate) noexcept {
        m_certificate = certificate;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::trustList(const QList<StorageId>& trust_list) noexcept {
        m_trust_list = trust_list;
        return *this;
    }

    ConnectionBuilder& ConnectionBuilder::revokedList(const QList<StorageId>& revoked_list) noexcept {
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

        const auto& storage_manager = Application::instance().getStorageManager();

        std::optional<ApplicationCertificate> app_cert;
        if (m_certificate.has_value()) {
            app_cert = storage_manager.getApplicationCertificate(*m_certificate);
            if (!app_cert.has_value()) {
                qCCritical(lc_builder) << "got invalid application certificate id, aborting...";
                return nullptr;
            }
        }

        const auto to_certs =
            std::views::transform([&storage_manager](StorageId cid) { return storage_manager.getCertificate(cid); })
            | std::views::filter([](const auto& opt) { return opt.has_value(); })
            | std::views::transform([](std::optional<QSslCertificate> cert) { return *std::move(cert); });

        constexpr auto to_qlist = [](auto&& range) {
            return QList(std::move_iterator{range.begin()}, std::move_iterator{range.end()});
        };

        const auto trust_list = to_qlist(m_trust_list | to_certs);
        if (m_trust_list.count() != trust_list.count()) {
            qCCritical(lc_builder) << "got invalid certificate id in trust list, aborting...";
            return nullptr;
        }

        const auto revoked_list = to_qlist(m_revoked_list | to_certs);
        if (m_revoked_list.count() != revoked_list.count()) {
            qCCritical(lc_builder) << "got invalid certificate id in revoked list, aborting...";
            return nullptr;
        }

        return new Connection(m_endpoint.value(), login, app_cert, trust_list, revoked_list, m_logger);
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

    const std::optional<StorageId>& ConnectionBuilder::getCertificate() const {
        return m_certificate;
    }

    const QList<StorageId>& ConnectionBuilder::getTrustList() const {
        return m_trust_list;
    }

    const QList<StorageId>& ConnectionBuilder::getRevokedList() const {
        return m_revoked_list;
    }
} // namespace magnesia::opcua_qt
