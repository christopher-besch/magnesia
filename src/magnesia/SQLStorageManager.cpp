#include "SQLStorageManager.hpp"

#include "HistoricServerConnection.hpp"
#include "Layout.hpp"
#include "StorageManager.hpp"
#include "database_types.hpp"
#include "opcua_qt/ApplicationCertificate.hpp"
#include "opcua_qt/abstraction/MessageSecurityMode.hpp"
#include "qt_version_check.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include <QJsonDocument>
#include <QLoggingCategory>
#include <QObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSsl>
#include <QSslCertificate>
#include <QSslKey>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypeTraits>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_sql_storage, "magnesia.storage.sql")
} // namespace

namespace {
    QVariant bind_optional(auto&& optional) {
        if (optional.has_value()) {
            return std::forward<decltype(optional)>(optional).value();
        }
        return {};
    }

    template<typename T>
    std::optional<T> get_optional(const QSqlQuery& query, const QString& name) {
        if (query.isNull(name)) {
            return std::nullopt;
        }
        return query.value(name).value<T>();
    };
} // namespace

namespace magnesia {
    SQLStorageManager::SQLStorageManager(const QString& db_location, QObject* parent)
        : StorageManager(parent), m_database{QSqlDatabase::addDatabase("QSQLITE")} {
        qCInfo(lc_sql_storage) << "using database" << db_location;
        m_database.setDatabaseName(db_location);

        if (!m_database.open()) {
            qCCritical(lc_sql_storage) << "Error: connection with database failed";
            terminate();
        }
        qCInfo(lc_sql_storage) << "Database: connection ok";

        // SQLite currently has foreign key support disabled by default. It has to be enabled per database connection:
        // https://www.sqlite.org/foreignkeys.html#fk_enable
        const QSqlQuery query{R"sql(PRAGMA foreign_keys = ON;)sql", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database failed to enable foreign_keys.", query);
            terminate();
        }

        migrate();
    }

    StorageId SQLStorageManager::storeCertificate(const QSslCertificate& cert) {
        Q_ASSERT(!cert.isNull());
        QSqlQuery query{m_database};
        query.prepare(R"sql(INSERT INTO Certificate VALUES (NULL, :pem, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":pem", cert.toPem());
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate storing failed.", query);
            terminate();
        }
        auto cert_id = getLastRowId();
        Q_EMIT certificateChanged(cert_id, StorageChange::Created);
        return cert_id;
    }

    StorageId SQLStorageManager::storeKey(const QSslKey& key) {
        Q_ASSERT(!key.isNull());
        QSqlQuery query{m_database};
        query.prepare(R"sql(INSERT INTO Key VALUES (NULL, :pem, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":pem", key.toPem());
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Key storing failed.", query);
            terminate();
        }
        auto key_id = getLastRowId();
        Q_EMIT keyChanged(key_id, StorageChange::Created);
        return key_id;
    }

    StorageId SQLStorageManager::storeApplicationCertificate(const opcua_qt::ApplicationCertificate& cert) {
        const auto cert_id = storeCertificate(cert.getCertificate());
        const auto key_id  = storeKey(cert.getPrivateKey());
        QSqlQuery  query{m_database};
        query.prepare(
            R"sql(INSERT INTO ApplicationCertificate VALUES (NULL, :cert_id, :key_id, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":cert_id", cert_id);
        query.bindValue(":key_id", key_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificate storing failed.", query);
            terminate();
        }
        auto app_cert_id = getLastRowId();
        Q_EMIT applicationCertificateChanged(app_cert_id, StorageChange::Created);
        return app_cert_id;
    }

    StorageId
    SQLStorageManager::storeHistoricServerConnection(const HistoricServerConnection& historic_server_connection) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
INSERT INTO HistoricServerConnection
VALUES (NULL, :server_url, :endpoint_url, :endpoint_security_policy_uri, :endpoint_message_security_mode, :username, :password, :certificate_id, :layout_id, :layout_group, :layout_domain, :last_used, CURRENT_TIMESTAMP);
                      )sql");
        query.bindValue(":server_url", historic_server_connection.server_url);
        query.bindValue(":endpoint_url", historic_server_connection.endpoint_url);
        query.bindValue(":endpoint_security_policy_uri", historic_server_connection.endpoint_security_policy_uri);
        query.bindValue(":endpoint_message_security_mode",
                        static_cast<qlonglong>(historic_server_connection.endpoint_message_security_mode));
        query.bindValue(":username", bind_optional(historic_server_connection.username));
        query.bindValue(":password", bind_optional(historic_server_connection.password));
        query.bindValue(":certificate_id", bind_optional(historic_server_connection.application_certificate_id));
        query.bindValue(":layout_id", bind_optional(historic_server_connection.last_layout_id));
        query.bindValue(":layout_group", bind_optional(historic_server_connection.last_layout_group));
        query.bindValue(":layout_domain", bind_optional(historic_server_connection.last_layout_domain));
        query.bindValue(":last_used", historic_server_connection.last_used);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection storing failed.", query);
            terminate();
        }
        auto historic_server_connection_id = getLastRowId();
        setHistoricServerConnectionTrustList(historic_server_connection_id,
                                             historic_server_connection.trust_list_certificate_ids);
        setHistoricServerConnectionRevokedList(historic_server_connection_id,
                                               historic_server_connection.revoked_list_certificate_ids);
        Q_EMIT historicServerConnectionChanged(historic_server_connection_id, StorageChange::Created);
        return historic_server_connection_id;
    }

    StorageId SQLStorageManager::storeLayout(const Layout& layout, const LayoutGroup& group, const Domain& domain) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
INSERT INTO Layout VALUES (NULL, :layout_group, :domain, :name, :json_data, CURRENT_TIMESTAMP);
                      )sql");
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.bindValue(":name", layout.name);
        query.bindValue(":json_data", layout.json_data.toJson(QJsonDocument::Compact));
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Layout storing failed.", query);
            terminate();
        }
        auto layout_id = getLastRowId();
        Q_EMIT layoutChanged(layout_id, group, domain, StorageChange::Created);
        return layout_id;
    }

    std::optional<QSslCertificate> SQLStorageManager::getCertificate(StorageId cert_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT pem FROM Certificate WHERE id = :id;)sql");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }

        auto certs = QSslCertificate::fromData(query.value("pem").toByteArray(), QSsl::EncodingFormat::Pem);
        // you may not store more than one certificate
        if (certs.size() != 1) {
            return {};
        }
        return certs.front();
    }

    std::optional<QSslKey> SQLStorageManager::getKey(StorageId key_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT pem FROM Key WHERE id = :id;)sql");
        query.bindValue(":id", key_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Key retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        // TODO: use actual key type
        QSslKey key{query.value("pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
        Q_ASSERT(!key.isNull());
        return key;
    }

    std::optional<opcua_qt::ApplicationCertificate>
    SQLStorageManager::getApplicationCertificate(StorageId cert_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT Certificate.pem, Key.pem
FROM ApplicationCertificate, Certificate, Key
WHERE ApplicationCertificate.certificate_id = Certificate.id
    AND ApplicationCertificate.key_id = Key.id
    AND ApplicationCertificate.id = :id;
                      )sql");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificate retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        auto certs = QSslCertificate::fromData(query.value("Certificate.pem").toByteArray(), QSsl::EncodingFormat::Pem);
        // you may not store more than one certificate
        if (certs.size() != 1) {
            return {};
        }
        // TODO: use actual key type
        const QSslKey key{query.value("Key.pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
        Q_ASSERT(!key.isNull());
        return opcua_qt::ApplicationCertificate{key, certs.front()};
    }

    std::optional<HistoricServerConnection>
    SQLStorageManager::getHistoricServerConnection(StorageId historic_server_connection_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT
    server_url,
    endpoint_url,
    endpoint_security_policy_uri,
    endpoint_message_security_mode,
    username,
    password,
    certificate_id,
    layout_id,
    layout_group,
    layout_domain,
    last_used
FROM HistoricServerConnection WHERE id = :id;
                      )sql");
        query.bindValue(":id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection retrieval failed.", query);
            terminate();
        }
        if (!query.next()) {
            return {};
        }
        return queryToHistoricServerConnection(query, historic_server_connection_id);
    }

    std::optional<Layout> SQLStorageManager::getLayout(StorageId layout_id, const LayoutGroup& group,
                                                       const Domain& domain) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT name, json_data FROM Layout WHERE id = :id AND layout_group = :layout_group AND domain = :domain;
                      )sql");
        query.bindValue(":id", layout_id);
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Layout retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return Layout{
            .name      = query.value("name").toString(),
            .json_data = QJsonDocument::fromJson(query.value("json_data").toString().toUtf8()),
        };
    }

    std::vector<std::pair<StorageId, QSslCertificate>> SQLStorageManager::getAllCertificates() const {
        QSqlQuery query{R"sql(SELECT id, pem FROM Certificate;)sql", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database all Certificate retrieval failed.", query);
            terminate();
        }

        std::vector<std::pair<StorageId, QSslCertificate>> certificates;
        while (query.next()) {
            const auto certs = QSslCertificate::fromData(query.value("pem").toByteArray(), QSsl::EncodingFormat::Pem);
            // you may not store more than one certificate
            if (certs.size() != 1) {
                return {};
            }
            certificates.emplace_back(query.value("id").toULongLong(), certs.front());
        }
        return certificates;
    }

    std::vector<std::pair<StorageId, QSslKey>> SQLStorageManager::getAllKeys() const {
        QSqlQuery query{R"sql(SELECT id, pem FROM Key;)sql", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database all Key retrieval failed.", query);
            terminate();
        }

        std::vector<std::pair<StorageId, QSslKey>> keys;
        while (query.next()) {
            // TODO: use actual key type
            QSslKey key{query.value("pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
            Q_ASSERT(!key.isNull());
            keys.emplace_back(query.value("id").toULongLong(), std::move(key));
        }
        return keys;
    }

    std::vector<std::pair<StorageId, opcua_qt::ApplicationCertificate>>
    SQLStorageManager::getAllApplicationCertificates() const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT ApplicationCertificate.id, Certificate.pem, Key.pem
FROM ApplicationCertificate, Certificate, Key
WHERE ApplicationCertificate.certificate_id = Certificate.id
    AND ApplicationCertificate.key_id = Key.id;
                      )sql");
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all ApplicationCertificate retrieval failed.", query);
            terminate();
        }

        std::vector<std::pair<StorageId, opcua_qt::ApplicationCertificate>> app_certificates;
        while (query.next()) {
            auto certs =
                QSslCertificate::fromData(query.value("Certificate.pem").toByteArray(), QSsl::EncodingFormat::Pem);
            if (certs.size() != 1) {
                qCCritical(lc_sql_storage)
                    << "application certificate's certificate pem data contains more than one certificate";
                terminate();
            }
            // TODO: use actual key type
            QSslKey key{query.value("Key.pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
            Q_ASSERT(!key.isNull());
            app_certificates.emplace_back(query.value("ApplicationCertificate.id").toULongLong(),
                                          opcua_qt::ApplicationCertificate{std::move(key), certs.front()});
        }
        return app_certificates;
    }

    std::vector<std::pair<StorageId, HistoricServerConnection>>
    SQLStorageManager::getAllHistoricServerConnections() const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT
    id AS historic_server_connection_id,
    server_url,
    endpoint_url,
    endpoint_security_policy_uri,
    endpoint_message_security_mode,
    username,
    password,
    certificate_id,
    layout_id,
    layout_group,
    layout_domain,
    last_used
FROM HistoricServerConnection;
                      )sql");
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all HistoricServerConnection retrieval failed.", query);
            terminate();
        }

        std::vector<std::pair<StorageId, HistoricServerConnection>> historic_connections;
        while (query.next()) {
            historic_connections.emplace_back(query.value("historic_server_connection_id").toULongLong(),
                                              queryToHistoricServerConnection(query));
        }
        return historic_connections;
    }

    std::vector<std::pair<StorageId, Layout>> SQLStorageManager::getAllLayouts(const LayoutGroup& group,
                                                                               const Domain&      domain) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT id, name, json_data FROM Layout WHERE layout_group = :layout_group AND domain = :domain;
                      )sql");
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all Layout retrieval failed.", query);
            terminate();
        }

        std::vector<std::pair<StorageId, Layout>> layouts;
        while (query.next()) {
            layouts.emplace_back(query.value("id").toULongLong(),
                                 Layout{
                                     .name      = query.value("name").toString(),
                                     .json_data = QJsonDocument::fromJson(query.value("json_data").toString().toUtf8()),
                                 });
        }
        return layouts;
    }

    void SQLStorageManager::deleteCertificate(StorageId cert_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM Certificate WHERE id = :id;)sql");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::deleteKey(StorageId key_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM Key WHERE id = :id;)sql");
        query.bindValue(":id", key_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Key deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::deleteApplicationCertificate(StorageId cert_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM ApplicationCertificate WHERE id = :id;)sql");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificate deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::deleteHistoricServerConnection(StorageId historic_server_connection_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM HistoricServerConnection WHERE id = :id;)sql");
        query.bindValue(":id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::deleteLayout(StorageId layout_id, const LayoutGroup& group, const Domain& domain) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
DELETE FROM Layout WHERE id = :id AND layout_group = :layout_group AND domain = :domain;
                      )sql");
        query.bindValue(":id", layout_id);
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Layout deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::setKV(const QString& key, const Domain& domain, const QString& value) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO KeyValue VALUES (:key, :domain, :value, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":key", key);
        query.bindValue(":domain", domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeyValue replace failed.", query);
            terminate();
        }
        Q_EMIT kvChanged(key, domain, StorageChange::Modified);
    }

    std::optional<QString> SQLStorageManager::getKV(const QString& key, const Domain& domain) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM KeyValue WHERE key = :key AND domain = :domain;)sql");
        query.bindValue(":key", key);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeyValue retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("value").toString();
    }

    void SQLStorageManager::deleteKV(const QString& key, const Domain& domain) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM KeyValue WHERE key = :key AND domain = :domain;)sql");
        query.bindValue(":key", key);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeyValue deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::resetSetting(const SettingKey& key) {
        // The specific setting (i.e. BooleanSetting) is deleted using SQLite's `ON DELETE CASCADE`.
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM Setting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Setting deletion failed.", query);
            terminate();
        }
        handleDeleteMonitor();
    }

    void SQLStorageManager::setGenericSetting(const SettingKey& key) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO Setting VALUES (:name, :domain, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Setting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setBooleanSetting(const SettingKey& key, bool value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO BooleanSetting VALUES (:name, :domain, :value);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database BooleanSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setStringSetting(const SettingKey& key, const QString& value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO StringSetting VALUES (:name, :domain, :value);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database StringSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setIntSetting(const SettingKey& key, std::int64_t value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO IntSetting VALUES (:name, :domain, :value);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        // qlonglong is guaranteed to be 64 bits: https://doc.qt.io/qt-6/qttypes.html#qlonglong-typedef
        query.bindValue(":value", static_cast<qlonglong>(value));
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database IntSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setDoubleSetting(const SettingKey& key, double value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO DoubleSetting VALUES (:name, :domain, :value);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database DoubleSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setEnumSetting(const SettingKey& key, const EnumSettingValue& value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO EnumSetting VALUES (:name, :domain, :value);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database EnumSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setCertificateSetting(const SettingKey& key, StorageId cert_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO CertificateSetting VALUES (:name, :domain, :cert_id);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":cert_id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database CertificateSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setKeySetting(const SettingKey& key, StorageId key_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO KeySetting VALUES (:name, :domain, :key_id);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":key_id", key_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeySetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setApplicationCertificateSetting(const SettingKey& key, StorageId cert_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO ApplicationCertificateSetting VALUES (:name, :domain, :cert_id);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":cert_id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificateSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setHistoricServerConnectionSetting(const SettingKey& key,
                                                               StorageId         historic_server_connection_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(
            R"sql(REPLACE INTO HistoricServerConnectionSetting VALUES (:name, :domain, :historic_server_connection_id);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":historic_server_connection_id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setLayoutSetting(const SettingKey& key, StorageId layout_id, const LayoutGroup& group) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO LayoutSetting VALUES (:name, :domain, :layout_id, :layout_group);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":layout_id", layout_id);
        query.bindValue(":layout_group", group);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database LayoutSetting replace failed.", query);
            terminate();
        }
    }

    std::optional<bool> SQLStorageManager::getBoolSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM BooleanSetting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database BooleanSetting retrieval failed.", query);
        }

        if (!query.next()) {
            return {};
        }
        return query.value("value").toBool();
    }

    std::optional<QString> SQLStorageManager::getStringSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM StringSetting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database StringSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("value").toString();
    }

    std::optional<std::int64_t> SQLStorageManager::getIntSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM IntSetting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database IntSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        // qlonglong is guaranteed to be 64 bits: https://doc.qt.io/qt-6/qttypes.html#qlonglong-typedef
        return query.value("value").toLongLong();
    }

    std::optional<double> SQLStorageManager::getDoubleSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM DoubleSetting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database DoubleSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("value").toDouble();
    }

    std::optional<EnumSettingValue> SQLStorageManager::getEnumSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT value FROM EnumSetting WHERE name = :name AND domain = :domain;)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database EnumSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return EnumSettingValue{query.value("value").toString()};
    }

    std::optional<QSslCertificate> SQLStorageManager::getCertificateSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT CertificateSetting.name,
    Certificate.pem
FROM CertificateSetting, Certificate
WHERE CertificateSetting.name = :name
    AND CertificateSetting.domain = :domain
    AND Certificate.id = CertificateSetting.cert_id;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database CertificateSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }

        auto certs = QSslCertificate::fromData(query.value("pem").toByteArray(), QSsl::EncodingFormat::Pem);
        // you may not store more than one certificate
        if (certs.size() != 1) {
            return {};
        }
        return certs.front();
    }

    std::optional<StorageId> SQLStorageManager::getCertificateSettingId(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT cert_id
FROM CertificateSetting
WHERE CertificateSetting.name = :name
    AND CertificateSetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database CertificateSetting id retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("cert_id").toULongLong();
    }

    std::optional<QSslKey> SQLStorageManager::getKeySetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT KeySetting.name,
    Key.pem
FROM KeySetting, Key
WHERE KeySetting.name = :name
    AND KeySetting.domain = :domain
    AND Key.id = KeySetting.key_id;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeySetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        // TODO: use actual key algorithm type
        QSslKey qkey{query.value("pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
        Q_ASSERT(!qkey.isNull());
        return qkey;
    }

    std::optional<StorageId> SQLStorageManager::getKeySettingId(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT key_id
FROM KeySetting
WHERE KeySetting.name = :name
    AND KeySetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeySetting id retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("key_id").toULongLong();
    }

    std::optional<opcua_qt::ApplicationCertificate>
    SQLStorageManager::getApplicationCertificateSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT Certificate.pem, Key.pem
FROM ApplicationCertificateSetting, ApplicationCertificate, Certificate, Key
WHERE ApplicationCertificateSetting.cert_id = ApplicationCertificate.id
    AND ApplicationCertificate.certificate_id = Certificate.id
    AND ApplicationCertificate.key_id = Key.id
    AND ApplicationCertificateSetting.name = :name
    AND ApplicationCertificateSetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificateSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }

        auto certs = QSslCertificate::fromData(query.value("Certificate.pem").toByteArray(), QSsl::EncodingFormat::Pem);
        // you may not store more than one certificate
        if (certs.size() != 1) {
            return {};
        }
        // TODO: use actual key type
        const QSslKey ssl_key{query.value("Key.pem").toByteArray(), QSsl::Rsa, QSsl::Pem};
        Q_ASSERT(!ssl_key.isNull());
        return opcua_qt::ApplicationCertificate{ssl_key, certs.front()};
    }

    std::optional<StorageId> SQLStorageManager::getApplicationCertificateSettingId(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT cert_id
FROM ApplicationCertificateSetting
WHERE ApplicationCertificateSetting.name = :name
    AND ApplicationCertificateSetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database ApplicationCertificateSetting id retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("cert_id").toULongLong();
    }

    std::optional<HistoricServerConnection>
    SQLStorageManager::getHistoricServerConnectionSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT
    HistoricServerConnection.id,
    HistoricServerConnection.server_url,
    HistoricServerConnection.endpoint_url,
    HistoricServerConnection.endpoint_security_policy_uri,
    HistoricServerConnection.endpoint_message_security_mode,
    HistoricServerConnection.username,
    HistoricServerConnection.password,
    HistoricServerConnection.certificate_id,
    HistoricServerConnection.layout_id,
    HistoricServerConnection.layout_group,
    HistoricServerConnection.layout_domain,
    HistoricServerConnection.last_used
AS historic_server_connection_id,
    server_url,
    endpoint_url,
    endpoint_security_policy_uri,
    endpoint_message_security_mode,
    username,
    password,
    certificate_id,
    layout_id,
    layout_group,
    layout_domain,
    last_used
FROM HistoricServerConnectionSetting, HistoricServerConnection
WHERE HistoricServerConnectionSetting.name = :name
    AND HistoricServerConnectionSetting.domain = :domain
    AND HistoricServerConnection.id = HistoricServerConnectionSetting.historic_server_connection_id;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return queryToHistoricServerConnection(query);
    }

    std::optional<StorageId> SQLStorageManager::getHistoricServerConnectionSettingId(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT historic_server_connection_id
FROM HistoricServerConnectionSetting
WHERE HistoricServerConnectionSetting.name = :name
    AND HistoricServerConnectionSetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionSetting id retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("historic_server_connection_id").toULongLong();
    }

    std::optional<Layout> SQLStorageManager::getLayoutSetting(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT Layout.name, Layout.json_data FROM LayoutSetting, Layout
WHERE LayoutSetting.name = :name
    AND LayoutSetting.domain = :domain
    AND Layout.id = LayoutSetting.layout_id
    AND Layout.layout_group = LayoutSetting.layout_group
    AND Layout.domain = LayoutSetting.domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database LayoutSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return Layout{
            .name      = query.value("Layout.name").toString(),
            .json_data = QJsonDocument::fromJson(query.value("Layout.json_data").toString().toUtf8()),
        };
    }

    std::optional<StorageId> SQLStorageManager::getLayoutSettingId(const SettingKey& key) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT layout_id
FROM LayoutSetting
WHERE LayoutSetting.name = :name
    AND LayoutSetting.domain = :domain;
                      )sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database LayoutSetting id retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("layout_id").toULongLong();
    }

    void SQLStorageManager::migrate() {
        // You may only **append** migration queries to this list.
        // If you want to remove a table don't just remove it's creation.
        // Instead append a query to drop that table.
        // This allows users to smoothly update.
        std::vector<QString> migrations{
            R"sql(
-- Every time a tuple in relation R is deleted the tuple with the DBRelation enum id of R and the identifier of the deleted tuple is appended to this relation.A
-- This is realized using SQL TRIGGERs for the relations with onChange Qt signals.
-- Once the application handled the event it is deleted from here.
CREATE TABLE TupleDeleteMonitor (
    primary_key_index INTEGER PRIMARY KEY,
    -- the relation from which a tuple has been deleted
    relation INTEGER NOT NULL,
    -- the id of the tuple that has been deleted, if it has an id as primary key
    id INTEGER,
    -- the key of the tuple that has been deleted, if it has a key as primary key
    key TEXT,
    -- the key of the tuple that has been deleted, if it has a group as primary key
    layout_group TEXT,
    -- the name of the tuple that has been deleted, if it has a name as primary key
    name TEXT,
    -- the domain of the tuple that has been deleted, if it has a domain as primary key
    domain TEXT
) STRICT;
)sql",
            R"sql(
CREATE TABLE Certificate (
    id INTEGER PRIMARY KEY,
    pem BLOB NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_certificate DELETE ON Certificate
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::Certificate)) + R"sql(, old.id, NULL, NULL, NULL, NULL);
END;
)sql",
            R"sql(
CREATE TABLE Key (
    id INTEGER PRIMARY KEY,
    pem BLOB NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_key DELETE ON Key
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::Key)) + R"sql(, old.id, NULL, NULL, NULL, NULL);
END;
)sql",
            R"sql(
CREATE TABLE ApplicationCertificate (
    id INTEGER PRIMARY KEY,
    certificate_id INTEGER NOT NULL,
    key_id INTEGER NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT ApplicationCertificate_TO_Certificate FOREIGN KEY (certificate_id)
        REFERENCES Certificate (id)
        ON DELETE CASCADE,
    CONSTRAINT ApplicationCertificate_TO_Key FOREIGN KEY (key_id)
        REFERENCES Key (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_application_certificate DELETE ON ApplicationCertificate
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::ApplicationCertificate))
                + R"sql(, old.id, NULL, NULL, NULL, NULL);
END;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnection (
    id INTEGER PRIMARY KEY,

    server_url TEXT NOT NULL,

    endpoint_url TEXT NOT NULL,
    endpoint_security_policy_uri TEXT NOT NULL,
    endpoint_message_security_mode INT NOT NULL,

    username TEXT,
    password TEXT,
    certificate_id INT,

    layout_id INT,
    layout_group TEXT,
    layout_domain TEXT,

    last_used TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT HistoricServerConnection_TO_ApplicationCertificate_FK FOREIGN KEY (certificate_id)
        REFERENCES ApplicationCertificate (id)
        ON DELETE CASCADE,
    CONSTRAINT HistoricServerConnection_TO_Layout_FK FOREIGN KEY (layout_id, layout_group, layout_domain)
        REFERENCES Layout (id, layout_group, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_historic_server_connection DELETE ON HistoricServerConnection
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::HistoricServerConnection))
                + R"sql(, old.id, NULL, NULL, NULL, NULL);
END;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnectionTrustList (
    historic_server_connection_id INTEGER NOT NULL,
    certificate_id INTEGER NOT NULL,
    --
    CONSTRAINT HistoricServerConnectionTrustList_PK PRIMARY KEY (historic_server_connection_id, certificate_id),
    CONSTRAINT HistoricServerConnectionTrustList_TO_HistoricServerConnection_FK FOREIGN KEY (historic_server_connection_id)
        REFERENCES HistoricServerConnection (id)
        ON DELETE CASCADE,
    CONSTRAINT HistoricServerConnectionTrustList_TO_Certificate_FK FOREIGN KEY (certificate_id)
        REFERENCES Certificate (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnectionRevokedList (
    historic_server_connection_id INTEGER NOT NULL,
    certificate_id INTEGER NOT NULL,
    --
    CONSTRAINT HistoricServerConnectionRevokedList_PK PRIMARY KEY (historic_server_connection_id, certificate_id),
    CONSTRAINT HistoricServerConnectionRevokedList_TO_HistoricServerConnection_FK FOREIGN KEY (historic_server_connection_id)
        REFERENCES HistoricServerConnection (id)
        ON DELETE CASCADE,
    CONSTRAINT HistoricServerConnectionRevokedList_TO_Certificate_FK FOREIGN KEY (certificate_id)
        REFERENCES Certificate (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TABLE Layout (
    -- unique for each (domain, layout_group) combination
    id INTEGER PRIMARY KEY,
    -- different types of layout in a domain (e.g. main-panel, side-panel)
    layout_group TEXT NOT NULL,
    -- same as the setting domain (e.g. DataViewer, HistoryViewer, ...)
    domain TEXT NOT NULL,
    -- the name can be set by the user and is not unique
    name TEXT NOT NULL,
    json_data BLOB NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    UNIQUE(id, layout_group, domain)
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_layout DELETE ON Layout
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::Layout))
                + R"sql(, old.id, NULL, old.layout_group, NULL, old.domain);
END;
)sql",
            R"sql(
CREATE TABLE KeyValue (
    key TEXT NOT NULL,
    domain TEXT NOT NULL,
    value TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT KeyValue_PK PRIMARY KEY (key, domain)
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_kv DELETE ON KeyValue
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::KeyValue)) + R"sql(, NULL, old.key, NULL, NULL, old.domain);
END;
)sql",
            R"sql(
-- always needs to be in company of a specific setting type
CREATE TABLE Setting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT Setting_PK PRIMARY KEY (name, domain)
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_setting DELETE ON Setting
BEGIN
    INSERT INTO TupleDeleteMonitor VALUES (NULL, )sql"
                + QString::number(qToUnderlying(DBRelation::Setting)) + R"sql(, NULL, NULL, NULL, old.name, old.domain);
END;
)sql",
            R"sql(
CREATE TABLE BooleanSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value INT NOT NULL,
    --
    CONSTRAINT BooleanSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT BooleanSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_bool_setting DELETE ON BooleanSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE StringSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value TEXT NOT NULL,
    --
    CONSTRAINT StringSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT StringSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_string_setting DELETE ON StringSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE IntSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value INT NOT NULL,
    --
    CONSTRAINT IntSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT IntSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_int_setting DELETE ON IntSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE DoubleSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value REAL NOT NULL,
    --
    CONSTRAINT DoubleSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT DoubleSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_double_setting DELETE ON DoubleSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE EnumSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value TEXT NOT NULL,
    --
    CONSTRAINT EnumSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT EnumSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_enum_setting DELETE ON EnumSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE CertificateSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    cert_id INT NOT NULL,
    --
    CONSTRAINT CertificateSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT CertificateSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT CertificateSetting_TO_Certificate_FK FOREIGN KEY (cert_id)
        REFERENCES Certificate (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_certificate_setting DELETE ON CertificateSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE ApplicationCertificateSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    cert_id INT NOT NULL,
    --
    CONSTRAINT ApplicationCertificateSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT ApplicationCertificateSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT ApplicationCertificateSetting_TO_ApplicationCertificate_FK FOREIGN KEY (cert_id)
        REFERENCES ApplicationCertificate (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_application_certificate_setting DELETE ON ApplicationCertificateSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE KeySetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    key_id INT NOT NULL,
    --
    CONSTRAINT KeySetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT KeySetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT KeySetting_TO_Key_FK FOREIGN KEY (key_id)
        REFERENCES Key (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_key_setting DELETE ON KeySetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnectionSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    historic_server_connection_id INT NOT NULL,
    --
    CONSTRAINT HistoricServerConnectionSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT HistoricServerConnectionSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT HistoricServerConnectionSetting_TO_HistoricServerConnection_FK FOREIGN KEY (historic_server_connection_id)
        REFERENCES HistoricServerConnection (id)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_historic_server_connection_setting DELETE ON HistoricServerConnectionSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
            R"sql(
CREATE TABLE LayoutSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    layout_id INT NOT NULL,
    layout_group TEXT NOT NULL,
    --
    CONSTRAINT LayoutSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT LayoutSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT LayoutSetting_TO_Layout_FK FOREIGN KEY (layout_id, layout_group, domain)
        REFERENCES Layout (id, layout_group, domain)
        ON DELETE CASCADE
) STRICT;
)sql",
            R"sql(
CREATE TRIGGER delete_layout_setting DELETE ON LayoutSetting
BEGIN
    DELETE FROM Setting WHERE name = old.name AND domain = old.domain;
END;
)sql",
        };

        // The user_version stores the index of the first migration that hasn't been executed yet.
        // See: https://www.sqlite.org/pragma.html#pragma_user_version
        //  > The user_version pragma will get or set the value of the user-version integer
        //  > at offset 60 in the database header. The user-version is an integer that is available
        //  > to applications to use however they want. SQLite makes no use of the user-version itself.
        QSqlQuery get_migration_version_query{R"sql(PRAGMA user_version;)sql", m_database};
        if (get_migration_version_query.lastError().isValid() || !get_migration_version_query.next()) {
            warnQuery("database user_version get failed.", get_migration_version_query);
            terminate();
        }
        unsigned int migration_version = get_migration_version_query.value(0).toUInt();

        while (migration_version < migrations.size()) {
            const QSqlQuery query{migrations[migration_version], m_database};
            if (query.lastError().isValid()) {
                qCCritical(lc_sql_storage) << migration_version << "failed";
                warnQuery("database migration failed", query);
                terminate();
            }
            auto next_migration_version = migration_version + 1;

            // Binding the migration_version fails because SQLite doesn't appear to support binds in pragmas.
            const QSqlQuery user_version_query{"PRAGMA user_version = " + QString::number(next_migration_version) + ";",
                                               m_database};
            if (user_version_query.lastError().isValid()) {
                warnQuery("database user_version change failed", user_version_query);
                terminate();
            }

            qCInfo(lc_sql_storage) << "Database: migration" << migration_version << "ok";
            migration_version = next_migration_version;
        }
        qCInfo(lc_sql_storage) << "Database: migration complete";

        // See: https://www.sqlite.org/pragma.html#pragma_integrity_check
        QSqlQuery integrity_check_query{R"sql(PRAGMA integrity_check;)sql", m_database};
        if (integrity_check_query.lastError().isValid() || !integrity_check_query.next()
            || integrity_check_query.value(0).toString() != "ok") {
            warnQuery("database integrity check failed.", integrity_check_query);
            terminate();
        }
        qCInfo(lc_sql_storage) << "Database: integrity check successful";
    }

    StorageId SQLStorageManager::getLastRowId() {
        QSqlQuery query{R"sql(SELECT last_insert_rowid();)sql", m_database};
        if (query.lastError().isValid() || !query.next()) {
            warnQuery("retrieving last row id from database failed.", query);
            terminate();
        }
        return query.value(0).toULongLong();
    }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity)
    void SQLStorageManager::handleDeleteMonitor() {
        QSqlQuery query{R"sql(
DELETE FROM TupleDeleteMonitor
RETURNING relation, id, key, layout_group, name, domain;
)sql",
                        m_database};
        if (query.lastError().isValid()) {
            warnQuery("retrieving TupleDeleteMonitor from database failed.", query);
            terminate();
        }

        // Qt doesn't guarantee multiple parallel queries working in parallel. This poses a problem when the slots
        // connected to the ...Changed signals call other functions on the SqlStorageManager that do a query on the
        // database, breaking the loop over the query results. Loading all entries eagerly fixes this by not relying on
        // the query after signaling the first changes.
        std::vector<std::tuple<DBRelation, unsigned long long, QString, QString, QString, QString>> deleted_tuples;
        while (query.next()) {
            deleted_tuples.emplace_back(static_cast<DBRelation>(query.value("relation").toUInt()),
                                        query.value("id").toULongLong(), query.value("key").toString(),
                                        query.value("layout_group").toString(), query.value("name").toString(),
                                        query.value("domain").toString());
        }

        for (const auto& [relation, id, key, layout_group, name, domain] : deleted_tuples) {
            switch (relation) {
                case DBRelation::Certificate:
                    qCDebug(lc_sql_storage) << "Certificate deleted";
                    Q_EMIT certificateChanged(id, StorageChange::Deleted);
                    continue;
                case DBRelation::Key:
                    qCDebug(lc_sql_storage) << "Key deleted";
                    Q_EMIT keyChanged(id, StorageChange::Deleted);
                    continue;
                case DBRelation::ApplicationCertificate:
                    qCDebug(lc_sql_storage) << "ApplicationCertificate deleted";
                    Q_EMIT applicationCertificateChanged(id, StorageChange::Deleted);
                    continue;
                case DBRelation::HistoricServerConnection:
                    qCDebug(lc_sql_storage) << "HistoricServerConnection deleted";
                    Q_EMIT historicServerConnectionChanged(id, StorageChange::Deleted);
                    continue;
                case DBRelation::Layout:
                    qCDebug(lc_sql_storage) << "Layout deleted";
                    Q_EMIT layoutChanged(id, layout_group, domain, StorageChange::Deleted);
                    continue;
                case DBRelation::KeyValue:
                    qCDebug(lc_sql_storage) << "KeyValue deleted";
                    Q_EMIT kvChanged(key, domain, StorageChange::Deleted);
                    continue;
                case DBRelation::Setting:
                    qCDebug(lc_sql_storage) << "Setting deleted";
                    Q_EMIT settingDeleted({.name = name, .domain = domain});
                    continue;
                case DBRelation::HistoricServerConnectionTrustList:
                case DBRelation::HistoricServerConnectionRevokedList:
                case DBRelation::BooleanSetting:
                case DBRelation::StringSetting:
                case DBRelation::IntSetting:
                case DBRelation::DoubleSetting:
                case DBRelation::EnumSetting:
                case DBRelation::CertificateSetting:
                case DBRelation::KeySetting:
                case DBRelation::HistoricServerConnectionSetting:
                case DBRelation::LayoutSetting:
                    // these should never appear
                    break;
            };
            qCCritical(lc_sql_storage) << "Invalid deletion type";
            terminate();
        }
    }

    void SQLStorageManager::warnQuery(const QString& message, const QSqlQuery& query) {
        qCWarning(lc_sql_storage) << "Error:" << message << "\nQuery:" << query.executedQuery()
                                  << "\nVariables:" << query.boundValues() << "\nError:" << query.lastError();
    }

    std::vector<StorageId>
    SQLStorageManager::getHistoricServerConnectionTrustList(StorageId historic_server_connection_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT certificate_id FROM HistoricServerConnectionTrustList WHERE historic_server_connection_id = :historic_server_connection_id;
                      )sql");
        query.bindValue(":id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionTrustList retrieval failed.", query);
            terminate();
        }

        std::vector<StorageId> certificate_ids;
        while (query.next()) {
            certificate_ids.push_back(query.value("certificate_id").toULongLong());
        }
        return certificate_ids;
    }

    std::vector<StorageId>
    SQLStorageManager::getHistoricServerConnectionRevokedList(StorageId historic_server_connection_id) const {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT certificate_id FROM HistoricServerConnectionRevokedList WHERE historic_server_connection_id = :historic_server_connection_id;
                      )sql");
        query.bindValue(":id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionRevokedList retrieval failed.", query);
            terminate();
        }

        std::vector<StorageId> certificate_ids;
        while (query.next()) {
            certificate_ids.push_back(query.value("certificate_id").toULongLong());
        }
        return certificate_ids;
    }

    void SQLStorageManager::deleteHistoricServerConnectionTrustList(StorageId historic_server_connection_id) {
        QSqlQuery query{m_database};
        query.prepare(
            R"sql(DELETE FROM HistoricServerConnectionTrustList WHERE historic_server_connection_id = :historic_server_connection_id;)sql");
        query.bindValue(":historic_server_connection_id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionTrustList deletion failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::deleteHistoricServerConnectionRevokedList(StorageId historic_server_connection_id) {
        QSqlQuery query{m_database};
        query.prepare(
            R"sql(DELETE FROM HistoricServerConnectionRevokedList WHERE historic_server_connection_id = :historic_server_connection_id;)sql");
        query.bindValue(":historic_server_connection_id", historic_server_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionRevokedList deletion failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setHistoricServerConnectionTrustList(StorageId historic_server_connection_id,
                                                                 std::span<const StorageId> certificates) {
        deleteHistoricServerConnectionRevokedList(historic_server_connection_id);
        for (auto cert_id : certificates) {
            QSqlQuery query{m_database};
            query.prepare(
                R"sql(INSERT INTO HistoricServerConnectionTrustList VALUES (:historic_server_connection_id, :cert_id);)sql");
            query.bindValue(":historic_server_connection_id", historic_server_connection_id);
            query.bindValue(":cert_id", cert_id);
            query.exec();
            if (query.lastError().isValid()) {
                warnQuery("database HistoricServerConnectionTrustList storing failed.", query);
                terminate();
            }
        }
    }

    void SQLStorageManager::setHistoricServerConnectionRevokedList(StorageId historic_server_connection_id,
                                                                   std::span<const StorageId> certificates) {
        deleteHistoricServerConnectionRevokedList(historic_server_connection_id);
        for (auto cert_id : certificates) {
            QSqlQuery query{m_database};
            query.prepare(
                R"sql(INSERT INTO HistoricServerConnectionRevokedList VALUES (:historic_server_connection_id, :cert_id);)sql");
            query.bindValue(":historic_server_connection_id", historic_server_connection_id);
            query.bindValue(":cert_id", cert_id);
            query.exec();
            if (query.lastError().isValid()) {
                warnQuery("database HistoricServerConnectionRevokedList storing failed.", query);
                terminate();
            }
        }
    }

    HistoricServerConnection
    SQLStorageManager::queryToHistoricServerConnection(const QSqlQuery& query,
                                                       StorageId        historic_server_connection_id) const {
        return {
            .server_url                   = query.value("server_url").toString(),
            .endpoint_url                 = query.value("endpoint_url").toString(),
            .endpoint_security_policy_uri = query.value("endpoint_security_policy_uri").toString(),
            .endpoint_message_security_mode =
                static_cast<opcua_qt::MessageSecurityMode>(query.value("endpoint_message_security_mode").toUInt()),
            .username                     = get_optional<QString>(query, "username"),
            .password                     = get_optional<QString>(query, "password"),
            .application_certificate_id   = get_optional<qulonglong>(query, "certificate_id"),
            .trust_list_certificate_ids   = getHistoricServerConnectionTrustList(historic_server_connection_id),
            .revoked_list_certificate_ids = getHistoricServerConnectionRevokedList(historic_server_connection_id),
            .last_layout_id               = get_optional<qulonglong>(query, "layout_id"),
            .last_layout_group            = get_optional<QString>(query, "layout_group"),
            .last_layout_domain           = get_optional<QString>(query, "layout_domain"),
            .last_used                    = query.value("last_used").toDateTime(),
        };
    }

    HistoricServerConnection SQLStorageManager::queryToHistoricServerConnection(const QSqlQuery& query) const {
        return queryToHistoricServerConnection(query, query.value("historic_server_connection_id").toULongLong());
    }
} // namespace magnesia
