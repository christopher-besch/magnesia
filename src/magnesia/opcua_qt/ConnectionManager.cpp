#include "ConnectionManager.hpp"

#include "../qt_version_check.hpp"
#include "Connection.hpp"
#include "ConnectionBuilder.hpp"
#include "Logger.hpp"
#include "abstraction/Endpoint.hpp"

#include <optional>

#include <open62541pp/AccessControl.h>

#include <QList>
#include <QObject>
#include <QString>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt {
    ConnectionManager::ConnectionManager(QObject* parent) : QObject(parent) {}

    ConnectionManager::~ConnectionManager() {
        closeAllConnections();
    }

    Connection* ConnectionManager::getConnection(int connection_id) noexcept {
        return m_connections.value(connection_id);
    }

    QList<int> ConnectionManager::getConnectionIds() const noexcept {
        return m_connections.keys();
    }

    void ConnectionManager::closeConnection(int connection_id) {
        if (m_connections.contains(connection_id)) {
            m_connections.value(connection_id)->close();
        }
    }

    void ConnectionManager::closeAllConnections() {
        for (Connection* connection : m_connections) {
            connection->close();
        }
    }

    Connection* ConnectionManager::createConnection(ConnectionBuilder& connection_builder) {
        std::optional<QString>  username = connection_builder.getUsername();
        std::optional<QString>  password = connection_builder.getPassword();
        std::optional<Endpoint> endpoint = connection_builder.getEndpoint();
        Logger*                 logger   = connection_builder.getLogger();
        Q_ASSERT(endpoint.has_value());
        Q_ASSERT(logger != nullptr);
        std::optional<opcua::Login> login;
        if (username.has_value() && password.has_value()) {
            login           = opcua::Login();
            login->username = username.value().toStdString();
            login->password = password.value().toStdString();
        }
        m_last_connection_id++;
        auto* connection = new Connection(endpoint.value(), login, connection_builder.getCertificate(),
                                          connection_builder.getTrustList(), connection_builder.getRevokedList(),
                                          logger, m_last_connection_id, this);
        m_connections.insert(m_last_connection_id, connection);
        return connection;
    }
} // namespace magnesia::opcua_qt
