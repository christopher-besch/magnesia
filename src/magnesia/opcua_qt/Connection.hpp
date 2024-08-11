#pragma once

#include "ApplicationCertificate.hpp"
#include "Logger.hpp"
#include "abstraction/AttributeId.hpp"
#include "abstraction/Endpoint.hpp"
#include "abstraction/NodeId.hpp"
#include "abstraction/Subscription.hpp"
#include "abstraction/node/Node.hpp"

#include <optional>

#include <open62541pp/AccessControl.h>
#include <open62541pp/Client.h>

#include <QList>
#include <QMutex>
#include <QObject>
#include <QSslCertificate>
#include <QTimer>
#include <QUrl>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    /**
     * @class Connection
     * @brief Models a OPC UA Connection
     */
    class Connection : public QObject {
        Q_OBJECT
      public:
        ~Connection() override;
        Connection(const Connection&)            = delete;
        Connection(Connection&&)                 = delete;
        Connection& operator=(const Connection&) = delete;
        Connection& operator=(Connection&&)      = delete;
        /**
         * @brief Constructor for a Connection.
         * The Connection is not connected until the connect method is used.
         *
         * @param endpoint the endpoint to connection is supposed to connect to
         * @param login the optional credentials to use for the connection
         * @param certificate the optional certificate
         * @param trust_list list of trusted certificates
         * @param revocation_list Certificate revocation lists (CRL)
         * @param logger the logger to use
         * @param parent the QObject parent for this connection (should be the connection manager)
         *
         * trust_list and revocation_list have no effect if there is no client certificate.
         */
        Connection(Endpoint endpoint, const std::optional<opcua::Login>& login,
                   const std::optional<ApplicationCertificate>& certificate, const QList<QSslCertificate>& trust_list,
                   const QList<QSslCertificate>& revocation_list, Logger* logger, QObject* parent = nullptr);
        /**
         * @brief Connects the underlying OPC UA client and poll for subscription updates asynchronously. The signal
         * connected gets emitted when the connection has been established.
         *
         * Don't do anything when the connection has already been established
         */
        void connectAndRun();
        /**
         * @brief gets the endpoint url with which the connection is connected
         *
         * @return a the endpoint's url
         */
        [[nodiscard]] QUrl getEndpointUrl() const noexcept;
        /**
         * @brief Returns the Root Node from the server
         *
         * @return Returns a Node Wrapper
         */
        [[nodiscard]] abstraction::Node* getRootNode();
        /**
         * @brief Gets a Node from a NodeId
         *
         * @param node_id NodeId that points to a Node
         *
         * @return Returns a Node Wrapper
         */
        [[nodiscard]] abstraction::Node* getNode(const abstraction::NodeId& node_id);
        /**
         * @brief Creates a Subscription Object
         *
         * @param node The Node that is to subscribe
         * @param attribute_ids Attributes that should be subscribed
         *
         * @return Returns a Subscription pointer
         */
        [[nodiscard]] abstraction::Subscription*
        createSubscription(abstraction::Node* node, const QList<abstraction::AttributeId>& attribute_ids);
        /**
         * @brief Stops polling for updates and disconnects the client
         */
        void close();

      signals:
        /**
         * @brief Gets emitted if the connection is connected
         */
        void connected();
        /**
         * @brief Gets emitted if the connection is disconnected
         */
        void disconnected();

      private:
        static opcua::Client constructClient(const std::optional<ApplicationCertificate>& certificate,
                                             const QList<QSslCertificate>&                trust_list,
                                             const QList<QSslCertificate>&                revocation_list);

        void connectSynchronouslyAndRun();

      private:
        opcua::Client               m_client;
        opcua_qt::Endpoint          m_server_endpoint;
        std::optional<opcua::Login> m_login;
        QTimer                      m_timer;
        // locked when connection is established
        QMutex m_connect_mutex;
    };
} // namespace magnesia::opcua_qt
