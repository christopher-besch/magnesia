#pragma once

#include "HistoricServerConnection.hpp"
#include "Layout.hpp"
#include "StorageManager.hpp"
#include "database_types.hpp"
#include "opcua_qt/ApplicationCertificate.hpp"

#include <cstdint>
#include <optional>
#include <utility>

#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSslCertificate>
#include <QSslKey>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {
    /**
     * The SQLStorageManager uses an SQLite database to store data.
     * Deletion triggers are used to get notified about cascading deletions.
     * Settings are implemented as a total partition.
     * I.e., there is a Setting and BooleanSetting tuple for each bool setting.
     * The deletion of the former uses ON DELETE CASCADE to delete the latter as well.
     * The other direction is implemented using deletion triggers.
     */
    class SQLStorageManager : public StorageManager {
        Q_OBJECT

      private:
        // These are used to identify in what relation a tuple has been deleted using TRIGGER ON DELETE.
        // NOTE: Don't change the values of the variants, they are independently persisted to the database.
        enum class DBRelation : uint8_t {
            Certificate                         = 0,
            Key                                 = 1,
            ApplicationCertificate              = 2,
            HistoricServerConnection            = 3,
            HistoricServerConnectionTrustList   = 4,
            HistoricServerConnectionRevokedList = 5,
            Layout                              = 6,
            KeyValue                            = 7,
            Setting                             = 8,
            BooleanSetting                      = 9,
            StringSetting                       = 10,
            IntSetting                          = 11,
            DoubleSetting                       = 12,
            EnumSetting                         = 13,
            CertificateSetting                  = 14,
            KeySetting                          = 15,
            HistoricServerConnectionSetting     = 16,
            LayoutSetting                       = 17,
        };

      public:
        explicit SQLStorageManager(const QString& db_location, QObject* parent = nullptr);

        StorageId storeCertificate(const QSslCertificate& cert) override;
        StorageId storeKey(const QSslKey& key) override;
        StorageId storeApplicationCertificate(const opcua_qt::ApplicationCertificate& cert) override;
        StorageId storeHistoricServerConnection(const HistoricServerConnection& historic_server_connection) override;
        StorageId storeLayout(const Layout& layout, const LayoutGroup& group, const Domain& domain) override;

        [[nodiscard]] std::optional<QSslCertificate> getCertificate(StorageId cert_id) const override;
        [[nodiscard]] std::optional<QSslKey>         getKey(StorageId key_id) const override;
        [[nodiscard]] std::optional<opcua_qt::ApplicationCertificate>
        getApplicationCertificate(StorageId cert_id) const override;
        [[nodiscard]] std::optional<HistoricServerConnection>
        getHistoricServerConnection(StorageId historic_server_connection_id) const override;
        [[nodiscard]] std::optional<Layout> getLayout(StorageId layout_id, const LayoutGroup& group,
                                                      const Domain& domain) const override;

        [[nodiscard]] QList<std::pair<StorageId, QSslCertificate>> getAllCertificates() const override;
        [[nodiscard]] QList<std::pair<StorageId, QSslKey>>         getAllKeys() const override;
        [[nodiscard]] QList<std::pair<StorageId, opcua_qt::ApplicationCertificate>>
        getAllApplicationCertificates() const override;
        [[nodiscard]] QList<std::pair<StorageId, HistoricServerConnection>>
                                                          getAllHistoricServerConnections() const override;
        [[nodiscard]] QList<std::pair<StorageId, Layout>> getAllLayouts(const LayoutGroup& group,
                                                                        const Domain&      domain) const override;

        void deleteCertificate(StorageId cert_id) override;
        void deleteKey(StorageId key_id) override;
        void deleteApplicationCertificate(StorageId cert_id) override;
        void deleteHistoricServerConnection(StorageId historic_server_connection_id) override;
        void deleteLayout(StorageId layout_id, const LayoutGroup& group, const Domain& domain) override;

        void setKV(const QString& key, const Domain& domain, const QString& value) override;
        [[nodiscard]] std::optional<QString> getKV(const QString& key, const Domain& domain) const override;
        void                                 deleteKV(const QString& key, const Domain& domain) override;

      private:
        void resetSetting(const SettingKey& key) override;
        void setBooleanSetting(const SettingKey& key, bool value) override;
        void setStringSetting(const SettingKey& key, const QString& value) override;
        void setIntSetting(const SettingKey& key, std::int64_t value) override;
        void setDoubleSetting(const SettingKey& key, double value) override;
        void setEnumSetting(const SettingKey& key, const EnumSettingValue& value) override;
        void setCertificateSetting(const SettingKey& key, StorageId cert_id) override;
        void setKeySetting(const SettingKey& key, StorageId key_id) override;
        void setApplicationCertificateSetting(const SettingKey& key, StorageId cert_id) override;
        void setHistoricServerConnectionSetting(const SettingKey& key,
                                                StorageId         historic_server_connection_id) override;
        void setLayoutSetting(const SettingKey& key, StorageId layout_id, const LayoutGroup& group) override;
        [[nodiscard]] std::optional<bool>             getBoolSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<QString>          getStringSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<std::int64_t>     getIntSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<double>           getDoubleSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<EnumSettingValue> getEnumSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<QSslCertificate>  getCertificateSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<StorageId>        getCertificateSettingId(const SettingKey& key) const override;
        [[nodiscard]] std::optional<QSslKey>          getKeySetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<StorageId>        getKeySettingId(const SettingKey& key) const override;
        [[nodiscard]] std::optional<opcua_qt::ApplicationCertificate>
                                               getApplicationCertificateSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<StorageId> getApplicationCertificateSettingId(const SettingKey& key) const override;
        [[nodiscard]] std::optional<HistoricServerConnection>
        getHistoricServerConnectionSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<StorageId>
                                            getHistoricServerConnectionSettingId(const SettingKey& key) const override;
        [[nodiscard]] std::optional<Layout> getLayoutSetting(const SettingKey& key) const override;
        [[nodiscard]] std::optional<StorageId> getLayoutSettingId(const SettingKey& key) const override;

      private:
        void migrate();
        /**
         * The id is chosen automatically by SQLite because of id INTEGER PRIMARY KEY.
         * This returns the last one used.
         */
        StorageId getLastRowId();
        /**
         * Every setting consists of a row in the generic Setting relation and a row in the specific setting (i.e.
         * BooleanSetting).
         * This updates the generic Setting's last_updated.
         */
        void setGenericSetting(const SettingKey& key);

        void handleDeleteMonitor();

        static void warnQuery(const QString& message, const QSqlQuery& query);

        [[nodiscard]] QList<StorageId>
        getHistoricServerConnectionTrustList(StorageId historic_server_connection_id) const;
        [[nodiscard]] QList<StorageId>
             getHistoricServerConnectionRevokedList(StorageId historic_server_connection_id) const;
        void deleteHistoricServerConnectionTrustList(StorageId historic_server_connection_id);
        void deleteHistoricServerConnectionRevokedList(StorageId historic_server_connection_id);
        void setHistoricServerConnectionTrustList(StorageId               historic_server_connection_id,
                                                  const QList<StorageId>& certificates);
        void setHistoricServerConnectionRevokedList(StorageId               historic_server_connection_id,
                                                    const QList<StorageId>& certificates);

        [[nodiscard]] HistoricServerConnection
        queryToHistoricServerConnection(const QSqlQuery& query, StorageId historic_server_connection_id) const;
        [[nodiscard]] HistoricServerConnection queryToHistoricServerConnection(const QSqlQuery& query) const;

      private:
        QSqlDatabase m_database;
    };
} // namespace magnesia
