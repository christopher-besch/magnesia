#include "SQLStorageManager.hpp"

#include "database_types.hpp"
#include "qt_version_check.hpp"
#include "terminate.hpp"

#include <optional>

#include <QList>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <qlogging.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#include <QtTypes>
#else
#include <QtDebug>
#include <QtGlobal>
#endif

namespace magnesia {

    SQLStorageManager::SQLStorageManager(const QString& db_location) {
        m_database = QSqlDatabase::addDatabase("QSQLITE");
        m_database.setDatabaseName(db_location);

        if (!m_database.open()) {
            qDebug("Error: connection with database failed");
            terminate();
        }
        qDebug("Database: connection ok");

        // This needs to be performed at every connection.
        const QSqlQuery query{"PRAGMA foreign_keys = ON;", m_database};
        if (query.lastError().isValid()) {
            warnQuery("database failed to enable foreign_keys.", query);
            terminate();
        }

        migrate();
    }

    StorageId SQLStorageManager::storeCertificate(Certificate cert) {
        QSqlQuery query{m_database};
        query.prepare("INSERT INTO Certificate VALUES (NULL, :name, :path, CURRENT_TIMESTAMP);");
        query.bindValue(":name", cert.name);
        query.bindValue(":path", cert.path_to_cert);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate storing failed.", query);
            terminate();
        }
        auto cert_id = getLastRowId();
        certificateChanged(cert_id);
        return cert_id;
    }

    StorageId SQLStorageManager::storeHistoricServerConnection(HistoricServerConnection historic_server_connection) {
        QSqlQuery query{m_database};
        query.prepare("INSERT INTO HistoricServerConnection VALUES (NULL, :address, :port, :cert_id, :layout_id, "
                      ":layout_group, :layout_domain, :last_used, CURRENT_TIMESTAMP);");
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
        historicConnectionChanged(server_con_id);
        return server_con_id;
    }

    StorageId SQLStorageManager::storeLayout(Layout layout, LayoutGroup group, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("INSERT INTO Layout VALUES (NULL, :layout_group, :domain, :name, :json_data, "
                      "CURRENT_TIMESTAMP);");
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
        layoutChanged(layout_id, group, domain);
        return layout_id;
    }

    std::optional<Certificate> SQLStorageManager::getCertificate(StorageId cert_id) {
        QSqlQuery query{m_database};
        query.prepare("SELECT name, path FROM Certificate WHERE id = :id;");
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
        query.prepare("SELECT address, port, cert_id, layout_id, layout_group, layout_domain, last_used FROM "
                      "HistoricServerConnection WHERE id = :id;");
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
            .port               = query.value("port").toInt(),
            .address            = query.value("address").toString(),
            .certificate_id     = query.value("cert_id").toULongLong(),
            .last_layout_id     = query.value("layout_id").toULongLong(),
            .last_layout_group  = query.value("layout_group").toString(),
            .last_layout_domain = query.value("layout_domain").toString(),
            .last_used          = query.value("last_used").toDateTime(),
        };
    }

    std::optional<Layout> SQLStorageManager::getLayout(StorageId layout_id, LayoutGroup group, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("SELECT name, json_data FROM Layout WHERE id = :id AND layout_group = "
                      ":layout_group AND domain = :domain;");
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
            .json_data = query.value("json_data").toString(),
        };
    }

    QList<Certificate> SQLStorageManager::getAllCertificates() {
        QSqlQuery query{m_database};
        query.prepare("SELECT name, path FROM Certificate;");
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all Certificate retrieval failed.", query);
            terminate();
        }

        QList<Certificate> certificates{};
        while (query.next()) {
            certificates.emplaceBack(Certificate{
                .name         = query.value("name").toString(),
                .path_to_cert = query.value("path").toString(),
            });
        }
        return certificates;
    }

    QList<HistoricServerConnection> SQLStorageManager::getAllHistoricServerConnections() {
        QSqlQuery query{m_database};
        query.prepare("SELECT address, port, cert_id, layout_id, layout_group, layout_domain, last_used FROM "
                      "HistoricServerConnection;");
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all HistoricServerConnection retrieval failed.", query);
            terminate();
        }

        QList<HistoricServerConnection> historic_connections{};
        while (query.next()) {
            historic_connections.emplaceBack(HistoricServerConnection{
                .port               = query.value("port").toInt(),
                .address            = query.value("address").toString(),
                .certificate_id     = query.value("cert_id").toULongLong(),
                .last_layout_id     = query.value("layout_id").toULongLong(),
                .last_layout_group  = query.value("layout_group").toString(),
                .last_layout_domain = query.value("layout_domain").toString(),
                .last_used          = query.value("last_used").toDateTime(),
            });
        }
        return historic_connections;
    }

    QList<Layout> SQLStorageManager::getAllLayouts(LayoutGroup group, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("SELECT name, json_data FROM Layout WHERE layout_group = "
                      ":layout_group AND domain = :domain;");
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database all Layout retrieval failed.", query);
            terminate();
        }

        QList<Layout> layouts{};
        while (query.next()) {
            layouts.emplaceBack(Layout{
                .name      = query.value("name").toString(),
                .json_data = query.value("json_data").toString(),
            });
        }
        return layouts;
    }

    void SQLStorageManager::deleteCertificate(StorageId cert_id) {
        QSqlQuery query{m_database};
        query.prepare("DELETE FROM Certificate WHERE id = :id;");
        query.bindValue(":id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Certificate deletion failed.", query);
            terminate();
        }
        certificateChanged(cert_id);
    }

    void SQLStorageManager::deleteHistoricServerConnection(StorageId historic_connection_id) {
        QSqlQuery query{m_database};
        query.prepare("DELETE FROM HistoricServerConnection WHERE id = :id;");
        query.bindValue(":id", historic_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnection deletion failed.", query);
            terminate();
        }
        historicConnectionChanged(historic_connection_id);
    }

    void SQLStorageManager::deleteLayout(StorageId layout_id, LayoutGroup group, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("DELETE FROM Layout WHERE id = :id AND layout_group = "
                      ":layout_group AND domain = :domain;");
        query.bindValue(":id", layout_id);
        query.bindValue(":layout_group", group);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Layout deletion failed.", query);
            terminate();
        }
        layoutChanged(layout_id, group, domain);
    }

    void SQLStorageManager::setKV(QString key, Domain domain, QString value) {
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO KeyValue VALUES (:key, :domain, :value, CURRENT_TIMESTAMP);");
        query.bindValue(":key", key);
        query.bindValue(":domain", domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeyValue replace failed.", query);
            terminate();
        }
        kvChanged(key, domain);
    }

    std::optional<QString> SQLStorageManager::getKV(QString key, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM KeyValue WHERE key = :key AND domain = :domain;");
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

    void SQLStorageManager::deleteKV(QString key, Domain domain) {
        QSqlQuery query{m_database};
        query.prepare("DELETE FROM KeyValue WHERE key = :key AND domain = :domain;");
        query.bindValue(":key", key);
        query.bindValue(":domain", domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database KeyValue deletion failed.", query);
            terminate();
        }
        kvChanged(key, domain);
    }

    void SQLStorageManager::resetSetting(SettingKey key) {
        // The specific setting (i.e. BooleanSetting) is deleted using SQLite's `ON DELETE CASCADE`.
        QSqlQuery query{m_database};
        query.prepare("DELETE FROM Setting WHERE name = :name AND domain = :domain;");
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
        query.prepare("REPLACE INTO Setting VALUES (:name, :domain, CURRENT_TIMESTAMP);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database Setting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setBooleanSetting(SettingKey key, bool value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO BooleanSetting VALUES (:name, :domain, :value);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database BooleanSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setStringSetting(SettingKey key, QString value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO StringSetting VALUES (:name, :domain, :value);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database StringSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setIntSetting(SettingKey key, int value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO IntSetting VALUES (:name, :domain, :value);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database IntSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setFloatSetting(SettingKey key, float value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO FloatSetting VALUES (:name, :domain, :value);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database FloatSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setEnumSetting(SettingKey key, EnumSettingValue value) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO EnumSetting VALUES (:name, :domain, :value);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":value", value);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database EnumSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setCertificateSetting(SettingKey key, StorageId cert_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO CertificateSetting VALUES (:name, :domain, :cert_id);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":cert_id", cert_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database CertificateSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setHistoricServerConnectionSetting(SettingKey key, StorageId historic_connection_id) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO HistoricServerConnectionSetting VALUES (:name, :domain, :server_con_id);");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.bindValue(":server_con_id", historic_connection_id);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database HistoricServerConnectionSetting replace failed.", query);
            terminate();
        }
    }

    void SQLStorageManager::setLayoutSetting(SettingKey key, StorageId layout_id, LayoutGroup group) {
        setGenericSetting(key);
        QSqlQuery query{m_database};
        query.prepare("REPLACE INTO LayoutSetting VALUES (:name, :domain, :layout_id, :layout_group);");
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

    std::optional<bool> SQLStorageManager::getBoolSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM BooleanSetting WHERE name = :name AND domain = :domain;");
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

    std::optional<QString> SQLStorageManager::getStringSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM StringSetting WHERE name = :name AND domain = :domain;");
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

    std::optional<int> SQLStorageManager::getIntSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM IntSetting WHERE name = :name AND domain = :domain;");
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
        return query.value("value").toInt();
    }

    std::optional<float> SQLStorageManager::getFloatSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM FloatSetting WHERE name = :name AND domain = :domain;");
        query.bindValue(":name", key.name);
        query.bindValue(":domain", key.domain);
        query.exec();
        if (query.lastError().isValid()) {
            warnQuery("database FloatSetting retrieval failed.", query);
            terminate();
        }

        if (!query.next()) {
            return {};
        }
        return query.value("value").toFloat();
    }

    std::optional<EnumSettingValue> SQLStorageManager::getEnumSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare("SELECT value FROM EnumSetting WHERE name = :name AND domain = :domain;");
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

    std::optional<Certificate> SQLStorageManager::getCertificateSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare(R"(
SELECT Certificate.name, Certificate.path FROM CertificateSetting, Certificate
WHERE CertificateSetting.name = :name AND CertificateSetting.domain = :domain
AND Certificate.id = CertificateSetting.cert_id;
                      )");
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

    std::optional<HistoricServerConnection> SQLStorageManager::getHistoricServerConnectionSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare(R"(
SELECT HistoricServerConnection.address, HistoricServerConnection.port, HistoricServerConnection.cert_id,
HistoricServerConnection.layout_id, HistoricServerConnection.layout_group, HistoricServerConnection.layout_domain,
HistoricServerConnection.last_used
FROM HistoricServerConnectionSetting, HistoricServerConnection
WHERE HistoricServerConnectionSetting.name = :name AND HistoricServerConnectionSetting.domain = :domain
AND HistoricServerConnection.id = HistoricServerConnectionSetting.server_con_id;
                      )");
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
            .port               = query.value("HistoricServerConnection.port").toInt(),
            .address            = query.value("HistoricServerConnection.address").toString(),
            .certificate_id     = query.value("HistoricServerConnection.cert_id").toULongLong(),
            .last_layout_id     = query.value("HistoricServerConnection.layout_id").toULongLong(),
            .last_layout_group  = query.value("HistoricServerConnection.layout_group").toString(),
            .last_layout_domain = query.value("HistoricServerConnection.layout_domain").toString(),
            .last_used          = query.value("HistoricServerConnection.last_used").toDateTime(),
        };
    }

    std::optional<Layout> SQLStorageManager::getLayoutSetting(SettingKey key) {
        QSqlQuery query{m_database};
        query.prepare(R"(
SELECT Layout.name, Layout.json_data FROM LayoutSetting, Layout
WHERE LayoutSetting.name = :name AND LayoutSetting.domain = :domain
AND Layout.id = LayoutSetting.layout_id AND Layout.layout_group = LayoutSetting.layout_group AND Layout.domain = LayoutSetting.domain;
                      )");
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
            .json_data = query.value("Layout.json_data").toString(),
        };
    }

    void SQLStorageManager::migrate() {
        // You may only **append** migration queries to this list.
        // If you want to remove a table don't just remove it's creation.
        // Instead append a query to drop that table.
        // This allows users to smoothly update.
        QList<QString> migrations{
            R"(
-- always needs to be in company of a specific setting type
CREATE TABLE Setting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT Setting_PK PRIMARY KEY (name, domain)
) STRICT;
)",
            R"(
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
)",
            R"(
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
)",
            R"(
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
)",
            R"(
CREATE TABLE FloatSetting (
    name TEXT NOT NULL,
    domain TEXT NOT NULL,
    value REAL NOT NULL,
    --
    CONSTRAINT FloatSetting_PK PRIMARY KEY (name, domain),
    CONSTRAINT FloatSetting_TO_Setting_FK FOREIGN KEY (name, domain)
        REFERENCES Setting (name, domain)
        ON DELETE CASCADE
) STRICT;
)",
            R"(
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
)",
            R"(
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
)",
            R"(
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
)",
            R"(
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
)",
            R"(
CREATE TABLE Certificate (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    path TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;
)",
            R"(
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
)",
            R"(
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
    UNIQUE(id, layout_group, domain)
) STRICT;
)",
            R"(
CREATE TABLE KeyValue (
    key TEXT NOT NULL,
    domain TEXT NOT NULL,
    value TEXT NOT NULL,
    last_updated TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    --
    CONSTRAINT KeyValue_PK PRIMARY KEY (key, domain)
) STRICT;
)",
        };

        // The user_version stores the index of the first migration that hasn't been executed yet.
        // See: https://www.sqlite.org/pragma.html#pragma_user_version
        //  > The user_version pragma will get or set the value of the user-version integer
        //  > at offset 60 in the database header. The user-version is an integer that is available
        //  > to applications to use however they want. SQLite makes no use of the user-version itself.
        QSqlQuery get_migration_version_query{"PRAGMA user_version;", m_database};
        if (get_migration_version_query.lastError().isValid() || !get_migration_version_query.next()) {
            warnQuery("database user_version get failed.", get_migration_version_query);
            terminate();
        }
        qsizetype migration_version = get_migration_version_query.value(0).toInt();

        while (migration_version < migrations.size()) {
            const QSqlQuery query = QSqlQuery{migrations[migration_version], m_database};
            if (query.lastError().isValid()) {
                qWarning() << migration_version << "failed";
                warnQuery("database migration failed", query);
                terminate();
            }
            auto next_migration_version = migration_version + 1;

            // Binding the migration_version fails because SQLite doesn't appear to support such binds.
            const QSqlQuery user_version_query =
                QSqlQuery{"PRAGMA user_version = " + QString::number(next_migration_version) + ";", m_database};
            if (user_version_query.lastError().isValid()) {
                warnQuery("database user_version change failed", user_version_query);
                terminate();
            }

            qDebug() << "Database: migration" << migration_version << "ok";
            migration_version = next_migration_version;
        }
        qDebug() << "Database: migration complete";

        // See: https://www.sqlite.org/pragma.html#pragma_integrity_check
        QSqlQuery integrity_check_query = QSqlQuery{"PRAGMA integrity_check;", m_database};
        if (integrity_check_query.lastError().isValid() || !integrity_check_query.next()
            || integrity_check_query.value(0).toString() != "ok") {
            warnQuery("database integrity_check failed.", integrity_check_query);
            terminate();
        }
        qDebug() << "Database: integrity_check ok";
    }

    StorageId SQLStorageManager::getLastRowId() {
        QSqlQuery query{"SELECT last_insert_rowid();", m_database};
        if (query.lastError().isValid() || !query.next()) {
            warnQuery("retrieving last row id from database failed.", query);
            terminate();
        }
        return static_cast<StorageId>(query.value(0).toInt());
    }

    void SQLStorageManager::warnQuery(const QString& message, const QSqlQuery& query) {
        qWarning() << "Error:" << message << "\nQuery:" << query.executedQuery()
                   << "\nVariables:" << query.boundValues() << "\nError:" << query.lastError();
    }

} // namespace magnesia
