#pragma once

#include "../database_types.hpp"
#include "ApplicationCertificate.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "abstraction/Endpoint.hpp"

#include <optional>

#include <open62541pp/Result.h>

#include <QList>
#include <QMutex>
#include <QObject>
#include <QSslCertificate>
#include <QString>
#include <QUrl>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    /**
     * @brief Class that is used to create a connection.
     */
    class ConnectionBuilder : public QObject {
        Q_OBJECT
      public:
        using QObject::QObject;
        /**
         * @brief Sets the url
         *
         * This function may be called only once.
         *
         * This is mandatory for a connection.
         *
         * @param url a QUrl
         */
        ConnectionBuilder& url(const QUrl& url) noexcept;
        /**
         * @brief Sets the endpoint to connect to.
         *
         * This function may be called only once.
         *
         * This is mandatory for a connection.
         *
         * @see ConnectionBuilder::findEndpoints()
         *
         * @param endpoint an Endpoint to connect to
         */
        ConnectionBuilder& endpoint(const Endpoint& endpoint) noexcept;
        /**
         * @brief Sets the logger to use for the connection.
         *
         * This function may be called only once.
         *
         * This is mandatory for a connection.
         *
         * @param logger a Logger method given to the connection
         */
        ConnectionBuilder& logger(Logger* logger) noexcept;
        /**
         * @brief Sets the username
         * Only if both a username and a password are provided, login with password and username is done.
         *
         * This function may be called only once.
         *
         * @param username a QString
         */
        ConnectionBuilder& username(const QString& username) noexcept;
        /**
         * @brief Sets the password
         * Only if both a username and a password are provided, login with password and username is done.
         *
         * This function may be called only once.
         *
         * @param password
         */
        ConnectionBuilder& password(const QString& password) noexcept;
        /**
         * @brief Sets a certificate.
         *
         * This function may be called only once.
         *
         * @param certificate a Certificate containing a private Key.
         */
        ConnectionBuilder& certificate(const StorageId& certificate) noexcept;
        /**
         * @brief Sets the trust list of certificates.
         * Only effective if a client certificate is selected.
         *
         * This function may be called only once.
         *
         * @param trust_list
         */
        ConnectionBuilder& trustList(const QList<StorageId>& trust_list) noexcept;
        /**
         * @brief Sets the revoked certificate list.
         * Only effective if a client certificate is selected.
         *
         * This function may be called only once.
         *
         * @param revoked_list
         */
        ConnectionBuilder& revokedList(const QList<StorageId>& revoked_list) noexcept;

        /**
         * @brief creates a connection from this builder by consuming the builder
         *
         * The endpoint and logger need to be set.
         *
         * This function may be called only once.
         *
         * @see ConnectionBuilder::endpoint()
         * @see ConnectionBuilder::logger()
         */
        Connection* build();
        /**
         * @brief Finds all endpoints from the url.
         *
         * This function may be called only once.
         *
         * The returned endpoints are valid as long as the ConnectionBuilder lives.
         *
         * The url needs to be set.
         *
         * This function may be called only once.
         *
         * @see ConnectionBuilder::url()
         */
        void findEndpoints();

        [[nodiscard]] const std::optional<QUrl>&      getUrl() const;
        [[nodiscard]] const std::optional<Endpoint>&  getEndpoint() const;
        [[nodiscard]] Logger*                         getLogger() const;
        [[nodiscard]] const std::optional<QString>&   getUsername() const;
        [[nodiscard]] const std::optional<QString>&   getPassword() const;
        [[nodiscard]] const std::optional<StorageId>& getCertificate() const;
        [[nodiscard]] const QList<StorageId>&         getTrustList() const;
        [[nodiscard]] const QList<StorageId>&         getRevokedList() const;

      signals:
        /**
         * @brief emits a list of endpoints from the url
         */
        void endpointsFound(opcua::Result<QList<Endpoint>> result);

      private:
        opcua::Result<QList<Endpoint>> findEndopintsSynchronously();

      private:
        std::optional<QUrl>      m_url;
        std::optional<Endpoint>  m_endpoint;
        Logger*                  m_logger{nullptr};
        std::optional<QString>   m_username;
        std::optional<QString>   m_password;
        std::optional<StorageId> m_certificate;
        QList<StorageId>         m_trust_list;
        QList<StorageId>         m_revoked_list;
        // this mutex protects m_url and m_endpoints
        QMutex m_get_endpoint_mutex;
    };
} // namespace magnesia::opcua_qt
