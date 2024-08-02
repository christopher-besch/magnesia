#pragma once

#include "database_types.hpp"
#include "opcua_qt/abstraction/MessageSecurityMode.hpp"

#include <optional>

#include <QDateTime>
#include <QList>
#include <QString>
#include <QUrl>

namespace magnesia {
    /**
     * A past connection with an OPC UA server.
     */
    struct HistoricServerConnection {
        QUrl server_url;

        QUrl                          endpoint_url;
        QString                       endpoint_security_policy_uri;
        opcua_qt::MessageSecurityMode endpoint_message_security_mode;

        std::optional<QString>   username;
        std::optional<QString>   password;
        std::optional<StorageId> certificate_id;
        std::optional<StorageId> private_key_certificate_id;
        QList<StorageId>         trust_list_certificate_ids;
        QList<StorageId>         revoked_list_certificate_ids;

        StorageId last_layout_id;
        QString   last_layout_group;
        Domain    last_layout_domain;

        QDateTime last_used;
    };
} // namespace magnesia
