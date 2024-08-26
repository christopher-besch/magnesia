#pragma once

#include "database_types.hpp"
#include "opcua_qt/abstraction/MessageSecurityMode.hpp"

#include <optional>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QUrl>

namespace magnesia {
    /**
     * @class HistoricServerConnection
     * @brief Struct representing a past connection with an OPC UA server.
     */
    struct HistoricServerConnection {
        /// The url used to connect to the server.
        QUrl server_url;

        /// Endpoint URL for the server.
        QUrl endpoint_url;

        /// The URI for SecurityPolicy to use when securing messages.
        QString endpoint_security_policy_uri;

        /// The type of security to apply to the messages.
        opcua_qt::MessageSecurityMode endpoint_message_security_mode{};

        /// Used username for the connection.
        std::optional<QString> username;

        /// Used password for the connection.
        std::optional<QString> password;

        /// Id of the certificate used for the connection.
        std::optional<StorageId> application_certificate_id;

        /// List of all trusted certificates.
        std::vector<StorageId> trust_list_certificate_ids;

        /// List of all revoked certificates.
        std::vector<StorageId> revoked_list_certificate_ids;

        /// Layout-Id last used while connected to the OPC UA server.
        std::optional<StorageId> last_layout_id;

        /// Group of the layout last used while connected to the OPC UA server.
        std::optional<QString> last_layout_group;

        /// Domain of the layout last used while connected to the OPC UA server.
        std::optional<Domain> last_layout_domain;

        /// Date of the last connection to the OPC UA server.
        QDateTime last_used;
    };
} // namespace magnesia
