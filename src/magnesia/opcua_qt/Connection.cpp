#include "Connection.hpp"

#include "../Application.hpp"
#include "../qt_version_check.hpp"
#include "ApplicationCertificate.hpp"
#include "Logger.hpp"
#include "abstraction/AttributeId.hpp"
#include "abstraction/Endpoint.hpp"
#include "abstraction/NodeId.hpp"
#include "abstraction/Subscription.hpp"
#include "abstraction/node/Node.hpp"

#include <algorithm>
#include <iterator>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <open62541pp/AccessControl.h>
#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/types/Builtin.h>

#include <QObject>
#include <QSslCertificate>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt {
    opcua::Client Connection::constructClient(const std::optional<ApplicationCertificate>& certificate,
                                              std::span<const QSslCertificate>             trust_list,
                                              std::span<const QSslCertificate>             revocation_list) {
        if (certificate.has_value()) {
            const auto              cert_der = certificate->getCertificate().toDer();
            const opcua::ByteString user_certificate(std::string_view{cert_der.begin(), cert_der.end()});

            const auto              key_pem = certificate->getPrivateKey().toPem();
            const opcua::ByteString private_key(std::string_view{key_pem.begin(), key_pem.end()});

            const auto cert_to_bytestring = [](const QSslCertificate& cert) {
                const auto der = cert.toDer();
                return opcua::ByteString(std::string_view{der.begin(), der.end()});
            };

            std::vector<opcua::ByteString> trust_list_bytestring;
            std::ranges::transform(trust_list, std::back_inserter(trust_list_bytestring), cert_to_bytestring);

            std::vector<opcua::ByteString> revocation_list_bytestring;
            std::ranges::transform(revocation_list, std::back_inserter(revocation_list_bytestring), cert_to_bytestring);

            return {user_certificate, private_key, trust_list_bytestring, revocation_list_bytestring};
        }
        return opcua::Client();
    }

    Connection::Connection(Endpoint endpoint, const std::optional<opcua::Login>& login,
                           const std::optional<ApplicationCertificate>& certificate,
                           std::span<const QSslCertificate>             trust_list,
                           std::span<const QSslCertificate> revocation_list, Logger* logger, QObject* parent)
        : QObject(parent), m_client(constructClient(certificate, trust_list, revocation_list)),
          m_server_endpoint(std::move(endpoint)), m_login(login) {
        Q_ASSERT(logger != nullptr);
        m_client.setLogger(logger->getOPCUALogger());
    }

    Connection::~Connection() {
        close();
    }

    void Connection::connectAndRun() {
        if (m_client.isConnected()) {
            return;
        }
        QThreadPool::globalInstance()->start([&] { connectSynchronouslyAndRun(); });
    }

    void Connection::connectSynchronouslyAndRun() {
        const std::unique_lock locker(m_connect_mutex);

        Q_ASSERT(!m_client.isRunning());
        m_client.setSecurityMode(static_cast<opcua::MessageSecurityMode>(m_server_endpoint.getSecurityMode()));
        if (m_login.has_value()) {
            m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString(), m_login.value());
        } else {
            m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString());
        }

        connect(this, &Connection::connected, this, [&] {
            m_timer.setInterval(
                static_cast<int>(Application::instance()
                                     .getSettingsManager()
                                     .getIntSetting({.name = "opcua_poll_intervall", .domain = "general"})
                                     .value()));
            connect(&m_timer, &QTimer::timeout, this, [&] { m_client.runIterate(0); });
            m_timer.start();
        });

        Q_EMIT connected();
    }

    QUrl Connection::getEndpointUrl() const noexcept {
        return m_server_endpoint.getEndpointUrl();
    }

    abstraction::Node* Connection::getRootNode() {
        return abstraction::Node::fromOPCUANode(m_client.getRootNode(), this);
    }

    abstraction::Node* Connection::getNode(const abstraction::NodeId& node_id) {
        return abstraction::Node::fromOPCUANode(m_client.getNode(node_id.handle()), this);
    }

    abstraction::Subscription* Connection::createSubscription(abstraction::Node*                        node,
                                                              std::span<const abstraction::AttributeId> attribute_ids) {
        auto* subscription = new abstraction::Subscription(m_client.createSubscription());
        for (const abstraction::AttributeId attribute_id : attribute_ids) {
            subscription->subscribeDataChanged(node, attribute_id);
        }
        return subscription;
    }

    void Connection::close() {
        m_timer.stop();
        m_client.stop();
        m_client.disconnect();
        Q_EMIT disconnected();
    }
} // namespace magnesia::opcua_qt
