#include "Connection.hpp"

#include "../qt_version_check.hpp"
#include "ApplicationCertificate.hpp"
#include "Logger.hpp"
#include "abstraction/AttributeId.hpp"
#include "abstraction/Endpoint.hpp"
#include "abstraction/NodeId.hpp"
#include "abstraction/Subscription.hpp"
#include "abstraction/node/Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/AccessControl.h>
#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/types/Builtin.h>

#include <QList>
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
                                              const QList<QSslCertificate>&                trust_list,
                                              const QList<QSslCertificate>&                revocation_list) {
        if (certificate.has_value()) {
            auto user_certificate      = opcua::ByteString(certificate.value().getCertificate().toDer().toStdString());
            auto private_key           = opcua::ByteString(certificate.value().getPrivateKey().toPem().toStdString());
            auto trust_list_bytestring = QList<opcua::ByteString>();
            for (const auto& cert : trust_list) {
                trust_list_bytestring.emplace_back(opcua::ByteString(cert.toDer().toStdString()));
            }
            auto revocation_list_bytestring = QList<opcua::ByteString>();
            for (const auto& cert : revocation_list) {
                revocation_list_bytestring.emplace_back(opcua::ByteString(cert.toDer().toStdString()));
            }
            return {user_certificate, private_key, trust_list_bytestring, revocation_list_bytestring};
        }
        return opcua::Client();
    }

    Connection::Connection(Endpoint endpoint, const std::optional<opcua::Login>& login,
                           const std::optional<ApplicationCertificate>& certificate,
                           const QList<QSslCertificate>& trust_list, const QList<QSslCertificate>& revocation_list,
                           Logger* logger, int connection_id, QObject* parent)
        : QObject(parent), m_client(constructClient(certificate, trust_list, revocation_list)), m_id(connection_id),
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
        m_connect_mutex.lock();
        Q_ASSERT(!m_client.isConnected());
        Q_ASSERT(!m_client.isRunning());
        m_client.setSecurityMode(static_cast<opcua::MessageSecurityMode>(m_server_endpoint.getSecurityMode()));
        if (m_login.has_value()) {
            m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString(), m_login.value());
        } else {
            m_client.connect(m_server_endpoint.getEndpointUrl().toString().toStdString());
        }

        connect(this, &Connection::connected, this, [&] {
            m_timer.setInterval(500); // NOLINT: cppcoreguidelines-avoid-magic-numbers
            connect(&m_timer, &QTimer::timeout, this, [&] { m_client.runIterate(0); });
            m_timer.start();
        });

        Q_EMIT connected();
        m_connect_mutex.unlock();
    }

    int Connection::getId() const noexcept {
        return m_id;
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

    abstraction::Subscription* Connection::createSubscription(abstraction::NodeId&             node_id,
                                                              QList<abstraction::AttributeId>& attribute_ids) {
        auto* subscription = new abstraction::Subscription(m_client.createSubscription());
        for (const abstraction::AttributeId attribute_id : attribute_ids) {
            subscription->subscribeDataChanged(node_id, attribute_id);
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
