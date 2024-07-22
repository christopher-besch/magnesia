#pragma once

#include "StorageManager.hpp"
#include "database_types.hpp"

#include <cstdint>
#include <optional>

#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {
    class SQLStorageManager : public StorageManager {
        Q_OBJECT

      public:
        explicit SQLStorageManager(const QString& db_location, QObject* parent = nullptr);

        StorageId storeCertificate(const Certificate& cert) override;
        StorageId storeHistoricServerConnection(const HistoricServerConnection& historic_server_connection) override;
        StorageId storeLayout(const Layout& layout, const LayoutGroup& group, const Domain& domain) override;

        std::optional<Certificate>              getCertificate(StorageId cert_id) override;
        std::optional<HistoricServerConnection> getHistoricServerConnection(StorageId historic_connection_id) override;
        std::optional<Layout> getLayout(StorageId layout_id, const LayoutGroup& group, const Domain& domain) override;

        QList<Certificate>              getAllCertificates() override;
        QList<HistoricServerConnection> getAllHistoricServerConnections() override;
        QList<Layout>                   getAllLayouts(const LayoutGroup& group, const Domain& domain) override;

        void deleteCertificate(StorageId cert_id) override;
        void deleteHistoricServerConnection(StorageId historic_connection_id) override;
        void deleteLayout(StorageId layout_id, const LayoutGroup& group, const Domain& domain) override;

        void                   setKV(const QString& key, const Domain& domain, const QString& value) override;
        std::optional<QString> getKV(const QString& key, const Domain& domain) override;
        void                   deleteKV(const QString& key, const Domain& domain) override;

      private:
        void resetSetting(const SettingKey& key) override;
        void setBooleanSetting(const SettingKey& key, bool value) override;
        void setStringSetting(const SettingKey& key, const QString& value) override;
        void setIntSetting(const SettingKey& key, std::int64_t value) override;
        void setDoubleSetting(const SettingKey& key, double value) override;
        void setEnumSetting(const SettingKey& key, const EnumSettingValue& value) override;
        void setCertificateSetting(const SettingKey& key, StorageId cert_id) override;
        void setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id) override;
        void setLayoutSetting(const SettingKey& key, StorageId layout_id, const LayoutGroup& group) override;
        std::optional<bool>                     getBoolSetting(const SettingKey& key) override;
        std::optional<QString>                  getStringSetting(const SettingKey& key) override;
        std::optional<std::int64_t>             getIntSetting(const SettingKey& key) override;
        std::optional<double>                   getDoubleSetting(const SettingKey& key) override;
        std::optional<EnumSettingValue>         getEnumSetting(const SettingKey& key) override;
        std::optional<Certificate>              getCertificateSetting(const SettingKey& key) override;
        std::optional<HistoricServerConnection> getHistoricServerConnectionSetting(const SettingKey& key) override;
        std::optional<Layout>                   getLayoutSetting(const SettingKey& key) override;

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

        static void warnQuery(const QString& message, const QSqlQuery& query);

      private:
        QSqlDatabase m_database;
    };
} // namespace magnesia
