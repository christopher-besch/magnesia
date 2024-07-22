#include "SQLStorageManager.hpp"

#include "StorageManager.hpp"
#include "database_types.hpp"
#include "qt_version_check.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <optional>

#include <QList>
#include <QObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#include <QtTypes>
#else
#include <QtDebug>
#include <QtGlobal>
#endif

namespace magnesia {
    SQLStorageManager::SQLStorageManager(const QString& db_location, QObject* parent)
        : StorageManager(parent), m_database{QSqlDatabase::addDatabase("QSQLITE")} {

        m_database.setDatabaseName(db_location);

        if (!m_database.open()) {
            qDebug() << "Error: connection with database failed";
            terminate();
        }
        qDebug() << "Database: connection ok";

        // SQLite currently has foreign key support disabled by default. It has to be enabled per database connection:
        // https://www.sqlite.org/foreignkeys.html#fk_enable
        const QSqlQuery query{R"sql(PRAGMA foreign_keys = ON;)sql", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database failed to enable foreign_keys.", query);
            terminate();
        }

        migrate();
    }

    StorageId SQLStorageManager::storeCertificate(const Certificate& cert) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(INSERT INTO Certificate VALUES (NULL, :name, :path, CURRENT_TIMESTAMP);)sql");
        query.bindValue(":name", cert.name);
        query.bindValue(":path", cert.path_to_cert);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate storing failed.", query);
            terminate();
        }
        auto cert_id = getLastRowId();
        Q_EMIT certificateChanged(cert_id);
        return cert_id;
    }

    StorageId
    SQLStorageManager::storeHistoricServerConnection(const HistoricServerConnection& historic_server_connection) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
INSERT INTO HistoricServerConnection
VALUES (NULL, :address, :port, :cert_id, :layout_id, :layout_group, :layout_domain, :last_used, CURRENT_TIMESTAMP);
                      )sql");
        query.bindValue(":address", historic_server_connection.address);
        query.bindValue(":port", historic_server_connection.port);
        query.bindValue(":cert_id", historic_server_connection.certificate_id);
        query.bindValue(":layout_id", historic_server_connection.last_layout_id);
        query.bindValue(":layout_group", historic_server_connection.last_layout_group);
        query.bindValue(":layout_domain", historic_server_connection.last_layout_domain);
        query.bindValue(":last_used", historic_server_connection.last_used);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection storing failed.", query);
            terminate();
        }
        auto server_con_id = getLastRowId();
        Q_EMIT historicConnectionChanged(server_con_id);
        return server_con_id;
    }

    StorageId SQLStorageManager::storeLayout(const Layout& layout, const LayoutGroup& group, const Domain& domain) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
INSERT INTO Layout VALUES (NULL, :layout_group, :domain, :name, :json_data, CURRENT_TIMESTAMP);
                      )sql");
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.bindValue(":name", layout.name);
        query.bindValue(":json_data", layout.json_data);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Layout storing failed.", query);
            terminate();
        }
        auto layout_id = getLastRowId();
        Q_EMIT layoutChanged(layout_id, group, domain);
        return layout_id;
    }

    std::optional<Certificate> SQLStorageManager::getCertificate(StorageId cert_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(SELECT name, path FROM Certificate WHERE id = :id;)sql");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return Certificate{
            .name         = query.value("name").toString(),
            .path_to_cert = query.value("path").toString(),
        };
    }

    std::optional<HistoricServerConnection>
    SQLStorageManager::getHistoricServerConnection(StorageId historic_connection_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT address, port, cert_id, layout_id, layout_group, layout_domain, last_used FROM HistoricServerConnection WHERE id = :id;
                      )sql");
        query.bindValue(":id", historic_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return HistoricServerConnection{
            .address            = query.value("address").toString(),
            .port               = query.value("port").toInt(),
            .certificate_id     = query.value("cert_id").toULongLong(),
            .last_layout_id     = query.value("layout_id").toULongLong(),
            .last_layout_group  = query.value("layout_group").toString(),
            .last_layout_domain = query.value("layout_domain").toString(),
            .last_used          = query.value("last_used").toDateTime(),
        };
    }

    std::optional<Layout> SQLStorageManager::getLayout(StorageId layout_id, const LayoutGroup& group,
                                                       const Domain& domain) {
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
            .json_data = query.value("json_data").toJsonDocument(),
        };
    }

    QList<Certificate> SQLStorageManager::getAllCertificates() {
        QSqlQuery query{R"sql(SELECT name, path FROM Certificate;)sql", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database all Certificate retrieval failed.", query);
            terminate();
        }

        QList<Certificate> certificates{};
        while (query.next()) {
            certificates.append({
                .name         = query.value("name").toString(),
                .path_to_cert = query.value("path").toString(),
            });
        }
        return certificates;
    }

    QList<HistoricServerConnection> SQLStorageManager::getAllHistoricServerConnections() {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT address, port, cert_id, layout_id, layout_group, layout_domain, last_used FROM HistoricServerConnection;
                      )sql");
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all HistoricServerConnection retrieval failed.", query);
            terminate();
        }

        QList<HistoricServerConnection> historic_connections{};
        while (query.next()) {
            historic_connections.append({
                .address            = query.value("address").toString(),
                .port               = query.value("port").toInt(),
                .certificate_id     = query.value("cert_id").toULongLong(),
                .last_layout_id     = query.value("layout_id").toULongLong(),
                .last_layout_group  = query.value("layout_group").toString(),
                .last_layout_domain = query.value("layout_domain").toString(),
                .last_used          = query.value("last_used").toDateTime(),
            });
        }
        return historic_connections;
    }

    QList<Layout> SQLStorageManager::getAllLayouts(const LayoutGroup& group, const Domain& domain) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT name, json_data FROM Layout WHERE layout_group = :layout_group AND domain = :domain;
                      )sql");
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all Layout retrieval failed.", query);
            terminate();
        }

        QList<Layout> layouts{};
        while (query.next()) {
            layouts.append({
                .name      = query.value("name").toString(),
                .json_data = query.value("json_data").toJsonDocument(),
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
        Q_EMIT certificateChanged(cert_id);
    }

    void SQLStorageManager::deleteHistoricServerConnection(StorageId historic_connection_id) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(DELETE FROM HistoricServerConnection WHERE id = :id;)sql");
        query.bindValue(":id", historic_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection deletion failed.", query);
            terminate();
        }
        Q_EMIT historicConnectionChanged(historic_connection_id);
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
        Q_EMIT layoutChanged(layout_id, group, domain);
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
        Q_EMIT kvChanged(key, domain);
    }

    std::optional<QString> SQLStorageManager::getKV(const QString& key, const Domain& domain) {
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
        Q_EMIT kvChanged(key, domain);
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

    void SQLStorageManager::setHistoricServerConnectionSetting(const SettingKey& key,
                                                               StorageId         historic_connection_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare(R"sql(REPLACE INTO HistoricServerConnectionSetting VALUES (:name, :domain, :server_con_id);)sql");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":server_con_id", historic_connection_id);
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

    std::optional<bool> SQLStorageManager::getBoolSetting(const SettingKey& key) {
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

    std::optional<QString> SQLStorageManager::getStringSetting(const SettingKey& key) {
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

    std::optional<std::int64_t> SQLStorageManager::getIntSetting(const SettingKey& key) {
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

    std::optional<double> SQLStorageManager::getDoubleSetting(const SettingKey& key) {
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

    std::optional<EnumSettingValue> SQLStorageManager::getEnumSetting(const SettingKey& key) {
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

    std::optional<Certificate> SQLStorageManager::getCertificateSetting(const SettingKey& key) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT Certificate.name,
    Certificate.path
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
        return Certificate{
            .name         = query.value("Certificate.name").toString(),
            .path_to_cert = query.value("Certificate.path").toString(),
        };
    }

    std::optional<HistoricServerConnection>
    SQLStorageManager::getHistoricServerConnectionSetting(const SettingKey& key) {
        QSqlQuery query{m_database};
        query.prepare(R"sql(
SELECT HistoricServerConnection.address,
    HistoricServerConnection.port,
    HistoricServerConnection.cert_id,
    HistoricServerConnection.layout_id,
    HistoricServerConnection.layout_group,
    HistoricServerConnection.layout_domain,
    HistoricServerConnection.last_used
FROM HistoricServerConnectionSetting, HistoricServerConnection
WHERE HistoricServerConnectionSetting.name = :name
    AND HistoricServerConnectionSetting.domain = :domain
    AND HistoricServerConnection.id = HistoricServerConnectionSetting.server_con_id;
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
        return HistoricServerConnection{
            .address            = query.value("HistoricServerConnection.address").toString(),
            .port               = query.value("HistoricServerConnection.port").toInt(),
            .certificate_id     = query.value("HistoricServerConnection.cert_id").toULongLong(),
            .last_layout_id     = query.value("HistoricServerConnection.layout_id").toULongLong(),
            .last_layout_group  = query.value("HistoricServerConnection.layout_group").toString(),
            .last_layout_domain = query.value("HistoricServerConnection.layout_domain").toString(),
            .last_used          = query.value("HistoricServerConnection.last_used").toDateTime(),
        };
    }

    std::optional<Layout> SQLStorageManager::getLayoutSetting(const SettingKey& key) {
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
            .json_data = query.value("Layout.json_data").toJsonDocument(),
        };
    }

    void SQLStorageManager::migrate() {
        // You may only **append** migration queries to this list.
        // If you want to remove a table don't just remove it's creation.
        // Instead append a query to drop that table.
        // This allows users to smoothly update.
        QList<QString> migrations{
            R"sql(
CREATE TABLE Certificate (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    path TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnection (
    id INTEGER PRIMARY KEY,
    address TEXT NOT NULL,
    port INT NOT NULL,
    cert_id INT NOT NULL,
    layout_id INT NOT NULL,
    layout_group TEXT NOT NULL,
    layout_domain TEXT NOT NULL,
    last_used TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT HistoricServerConnection_TO_Certificate_FK FOREIGN KEY (cert_id)
        REFERENCES Certificate (id),
    CONSTRAINT HistoricServerConnection_TO_Layout_FK FOREIGN KEY (layout_id, layout_group, layout_domain)
        REFERENCES Layout (id, layout_group, domain)
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
    json_data TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    UNIQUE(id, layout_group, domain)
) STRICT;
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
) STRICT;
)sql",
            R"sql(
CREATE TABLE HistoricServerConnectionSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    server_con_id INT NOT NULL,
    --
    CONSTRAINT HistoricServerConnectionSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT HistoricServerConnectionSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE,
    CONSTRAINT HistoricServerConnectionSetting_TO_HistoricServerConnection_FK FOREIGN KEY (server_con_id)
        REFERENCES HistoricServerConnection (id)
) STRICT;
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
) STRICT;
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
                qWarning() << migration_version << "failed";
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

            qDebug() << "Database: migration" << migration_version << "ok";
            migration_version = next_migration_version;
        }
        qDebug() << "Database: migration complete";

        // See: https://www.sqlite.org/pragma.html#pragma_integrity_check
        QSqlQuery integrity_check_query{R"sql(PRAGMA integrity_check;)sql", m_database};
        if (integrity_check_query.lastError().isValid() || !integrity_check_query.next()
            || integrity_check_query.value(0).toString() != "ok") {
            warnQuery("database integrity check failed.", integrity_check_query);
            terminate();
        }
        qDebug() << "Database: integrity check successful";
    }

    StorageId SQLStorageManager::getLastRowId() {
        QSqlQuery query{R"sql(SELECT last_insert_rowid();)sql", m_database};
        if (query.lastError().isValid() || !query.next()) {
            warnQuery("retrieving last row id from database failed.", query);
            terminate();
        }
        return query.value(0).toULongLong();
    }

    void SQLStorageManager::warnQuery(const QString& message, const QSqlQuery& query) {
        qWarning() << "Error:" << message << "\nQuery:" << query.executedQuery()
                   << "\nVariables:" << query.boundValues() << "\nError:" << query.lastError();
    }
} // namespace magnesia
