#pragma once

#include "ApplicationCertificate.hpp"
#include "Logger.hpp"
#include "abstraction/AttributeId.hpp"
#include "abstraction/Endpoint.hpp"
#include "abstraction/NodeId.hpp"
#include "abstraction/Subscription.hpp"
#include "abstraction/node/Node.hpp"
#include "qt_version_check.hpp"

#include <map>
#include <optional>
#include <span>

#include <open62541pp/AccessControl.h>
#include <open62541pp/Client.h>

#include <QObject>
#include <QSslCertificate>
#include <QTimer>
#include <QUrl>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtClassHelperMacros>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt {
    /**
     * @class Connection
     * @brief Models a OPC UA Connection
     */
    class Connection : public QObject {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Connection)

      public:
        ~Connection() override;
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
                   const std::optional<ApplicationCertificate>& certificate,
                   std::span<const QSslCertificate> trust_list, std::span<const QSslCertificate> revocation_list,
                   Logger* logger, QObject* parent = nullptr);
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
         * @return Returns a Node Wrapper or nullopt if an error occurs
         */
        [[nodiscard]] std::optional<abstraction::Node*> getRootNode();
        /**
         * @brief Gets a Node from a NodeId
         *
         * @param node_id NodeId that points to a Node
         *
         * @return Returns a Node Wrapper or nullopt if an error occurs
         */
        [[nodiscard]] std::optional<abstraction::Node*> getNode(const abstraction::NodeId& node_id);
        /**
         * @brief Creates a Subscription Object
         *
         * @param node The Node that is to subscribe
         * @param attribute_ids Attributes that should be subscribed
         *
         * @return Returns a Subscription pointer
         */
        [[nodiscard]] abstraction::Subscription*
        createSubscription(abstraction::Node* node, std::span<const abstraction::AttributeId> attribute_ids);
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
                                             std::span<const QSslCertificate>             trust_list,
                                             std::span<const QSslCertificate>             revocation_list);

      private:
        opcua::Client               m_client;
        opcua_qt::Endpoint          m_server_endpoint;
        std::optional<opcua::Login> m_login;
        QTimer                      m_timer;

        abstraction::Node*                                m_root_node{};
        std::map<abstraction::NodeId, abstraction::Node*> m_nodes;
    };
} // namespace magnesia::opcua_qt
