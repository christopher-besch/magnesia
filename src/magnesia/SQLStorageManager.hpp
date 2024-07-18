#pragma once
#include "StorageManager.hpp"
#include "database_types.hpp"

#include <optional>

#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QStringView>
#include <qtmetamacros.h>

namespace magnesia {
    class SQLStorageManager : public StorageManager {
        Q_OBJECT

      public:
        explicit SQLStorageManager(const QString& db_location);

        // return the id used
        StorageId storeCertificate(Certificate cert) override;
        StorageId storeHistoricServerConnection(HistoricServerConnection historic_server_connection) override;
        StorageId storeLayout(Layout layout, LayoutGroup group, Domain domain) override;

        std::optional<Certificate>              getCertificate(StorageId cert_id) override;
        std::optional<HistoricServerConnection> getHistoricServerConnection(StorageId historic_connection_id) override;
        std::optional<Layout> getLayout(StorageId layout_id, LayoutGroup group, Domain domain) override;

        // e.g. used to query all possible options for a setting
        QList<Certificate>              getAllCertificates() override;
        QList<HistoricServerConnection> getAllHistoricServerConnections() override;
        QList<Layout>                   getAllLayouts(LayoutGroup group, Domain domain) override;

        void deleteCertificate(StorageId cert_id) override;
        void deleteHistoricServerConnection(StorageId historic_connection_id) override;
        void deleteLayout(StorageId layout_id, LayoutGroup group, Domain domain) override;

        void                   setKV(QString key, Domain domain, QString value) override;
        std::optional<QString> getKV(QString key, Domain domain) override;
        void                   deleteKV(QString key, Domain domain) override;

        // only the SettingsManager may use these
      private:
        void resetSetting(SettingKey key) override;
        // The SettingsManager needs to ensure that the type of a setting doesn't change.
        // Otherwise you could have two settings with the same name and domain for different types.
        void setBooleanSetting(SettingKey key, bool value) override;
        void setStringSetting(SettingKey key, QString value) override;
        void setIntSetting(SettingKey key, int value) override;
        void setFloatSetting(SettingKey key, float value) override;
        void setEnumSetting(SettingKey key, EnumSettingValue value) override;
        void setCertificateSetting(SettingKey key, StorageId cert_id) override;
        void setHistoricServerConnectionSetting(SettingKey key, StorageId historic_connection_id) override;
        void setLayoutSetting(SettingKey key, StorageId layout_id, LayoutGroup group) override;
        std::optional<bool>                     getBoolSetting(SettingKey key) override;
        std::optional<QString>                  getStringSetting(SettingKey key) override;
        std::optional<int>                      getIntSetting(SettingKey key) override;
        std::optional<float>                    getFloatSetting(SettingKey key) override;
        std::optional<EnumSettingValue>         getEnumSetting(SettingKey key) override;
        std::optional<Certificate>              getCertificateSetting(SettingKey key) override;
        std::optional<HistoricServerConnection> getHistoricServerConnectionSetting(SettingKey key) override;
        std::optional<Layout>                   getLayoutSetting(SettingKey key) override;

      private:
        void migrate();
        /**
         * The id is chosen automatically by SQLite
         * because of id INTEGER PRIMARY KEY.
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
