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

#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <open62541/types.h>
#include <open62541pp/AccessControl.h>
#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/types/Builtin.h>

#include <QLoggingCategory>
#include <QObject>
#include <QSslCertificate>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypeTraits>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_opcua_connection, "magnesia.opcua.connection")
} // namespace

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

            auto              trust_list_view = std::views::transform(trust_list, cert_to_bytestring);
            const std::vector trust_list_bytestring(trust_list_view.begin(), trust_list_view.end());

            auto              revocation_list_view = std::views::transform(revocation_list, cert_to_bytestring);
            const std::vector revocation_list_bytestring(revocation_list_view.begin(), revocation_list_view.end());

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

        Q_ASSERT(!m_client.isRunning());

        try {
            m_client.setSecurityMode(static_cast<opcua::MessageSecurityMode>(m_server_endpoint.getSecurityMode()));
            if (m_login.has_value()) {
                m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString(), m_login.value());
            } else {
                m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString());
            }
        } catch (const opcua::BadStatus& status) {
            qCWarning(lc_opcua_connection) << "Failed to connect to server:" << status.what();
            return;
        }

        const auto interval = Application::instance().getSettingsManager().getIntSetting(
            {.name = "opcua_poll_intervall", .domain = "general"});
        // Can only be nullopt if the setting was never defined or is of the wrong type. Both should never happen.
        Q_ASSERT(interval);

        m_timer.setInterval(static_cast<int>(interval.value()));
        connect(&m_timer, &QTimer::timeout, this, [this] { m_client.runIterate(0); });
        m_timer.start();

        Q_EMIT connected();
    }

    QUrl Connection::getEndpointUrl() const noexcept {
        return m_server_endpoint.getEndpointUrl();
    }

    std::optional<abstraction::Node*> Connection::getRootNode() {
        if (m_root_node == nullptr) {
            try {
                m_root_node = abstraction::Node::fromOPCUANode(m_client.getRootNode(), this);
            } catch (const opcua::BadStatus& status) {
                qCWarning(lc_opcua_connection) << "Failed to get root node:" << status.what();
                return std::nullopt;
            }
        }
        return m_root_node;
    }

    std::optional<abstraction::Node*> Connection::getNode(const abstraction::NodeId& node_id) {
        if (auto node = m_nodes.find(node_id); node != m_nodes.end()) {
            return node->second;
        }
        try {
            return m_nodes[node_id] = abstraction::Node::fromOPCUANode(m_client.getNode(node_id.handle()), this);
        } catch (const opcua::BadStatus& status) {
            qCWarning(lc_opcua_connection) << "Failed to get node:" << status.what();
            return std::nullopt;
        }
    }

    abstraction::Subscription* Connection::createSubscription(abstraction::Node*                        node,
                                                              std::span<const abstraction::AttributeId> attribute_ids) {
        abstraction::Subscription* subscription{};
        try {
            subscription = new abstraction::Subscription(m_client.createSubscription());
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
        for (const abstraction::AttributeId attribute_id : attribute_ids) {
            try {
                subscription->subscribeDataChanged(node, attribute_id);
            } catch (const opcua::BadStatus& status) {
                if (status.code() == UA_STATUSCODE_BADNOTSUPPORTED) {
                    qCInfo(lc_opcua_connection) << "Failed to subscribe to attribute" << qToUnderlying(attribute_id)
                                                << "reason:" << status.what();
                } else {
                    qCWarning(lc_opcua_connection) << "Failed to subscribe to attribute" << qToUnderlying(attribute_id)
                                                   << "reason:" << status.what();
                }
            }
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
