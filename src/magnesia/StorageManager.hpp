#pragma once

#include "database_types.hpp"

#include <optional>

#include <QList>
#include <QObject>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {
    // properly defined in SettingsManager.hpp
    class SettingsManager;

    /**
     * Base class for handling persistent storage.
     */
    class StorageManager : public QObject {
        Q_OBJECT

      protected:
        using QObject::QObject;

      public:
        /**
         * Store a Certificate in the database.
         *
         * Exit the application on error.
         *
         * @param cert The Certificate to store.
         * @return the new database id of the Certificate.
         */
        virtual StorageId storeCertificate(const Certificate& cert) = 0;
        /**
         * Store a HistoricServerConnection in the database.
         *
         * Exit the application on error.
         *
         * @param historic_server_connection The HistoricServerConnection to store.
         * @return the new database id of the HistoricServerConnection.
         */
        virtual StorageId storeHistoricServerConnection(const HistoricServerConnection& historic_server_connection) = 0;
        /**
         * Store a Layout in the database.
         *
         * Exit the application on error.
         *
         * @param group The LayoutGroup this layout belongs to.
         * @param domain The Domain this layout belongs to.
         * @param layout The Layout to store.
         * @return the new database id of the Layout.
         */
        virtual StorageId storeLayout(const Layout& layout, const LayoutGroup& group, const Domain& domain) = 0;

        /**
         * Retrieve a Certificate from the database.
         *
         * Exit the application on error.
         *
         * @param cert_id The id the Certificate is stored under.
         * @return the Certificate or nullopt when not found.
         */
        virtual std::optional<Certificate> getCertificate(StorageId cert_id) = 0;
        /**
         * Retrieve a HistoricServerConnection from the database.
         *
         * Exit the application on error.
         *
         * @param historic_connection_id The id the HistoricServerConnection is stored under.
         * @return the HistoricServerConnection or nullopt when not found.
         */
        virtual std::optional<HistoricServerConnection>
        getHistoricServerConnection(StorageId historic_connection_id) = 0;
        /**
         * Retrieve a Layout from the database.
         *
         * Exit the application on error.
         *
         * @param layout_id The id the Layout is stored under.
         * @param group The LayoutGroup this layout belongs to.
         * @param domain The Domain this layout belongs to.
         * @return the Layout or nullopt when not found.
         */
        virtual std::optional<Layout> getLayout(StorageId layout_id, const LayoutGroup& group,
                                                const Domain& domain) = 0;

        /**
         * Retrieve all Certificates from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all Certificates.
         */
        virtual QList<Certificate> getAllCertificates() = 0;
        /**
         * Retrieve all HistoricServerConnections from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all HistoricServerConnections.
         */
        virtual QList<HistoricServerConnection> getAllHistoricServerConnections() = 0;
        /**
         * Retrieve all Layouts from the database for a specified group and domain.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @param group The LayoutGroup the layouts belong to.
         * @param domain The Domain the layouts belong to.
         * @return a list of all Layouts with the specified group and domain.
         */
        virtual QList<Layout> getAllLayouts(const LayoutGroup& group, const Domain& domain) = 0;

        /**
         * Delete the Certificate with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param cert_id The id of the Certificate.
         */
        virtual void deleteCertificate(StorageId cert_id) = 0;
        /**
         * Delete the HistoricServerConnection with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param historic_connection_id The id of the HistoricServerConnection.
         */
        virtual void deleteHistoricServerConnection(StorageId historic_connection_id) = 0;
        /**
         * Delete the Layout with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param layout_id The id the Layout is stored under.
         * @param group The LayoutGroup this layout belongs to.
         * @param domain The Domain this layout belongs to.
         */
        virtual void deleteLayout(StorageId layout_id, const LayoutGroup& group, const Domain& domain) = 0;

        /**
         * Set or update the value for the specified key in a specified domain.
         *
         * Exit the application on error.
         *
         * @param key the key to set the value for.
         * @param domain the key-value pair belongs to.
         * @param value the new value.
         */
        virtual void setKV(const QString& key, const Domain& domain, const QString& value) = 0;
        /**
         * Get the value for the specified key in a specified domain.
         *
         * Exit the application on error. Don't exit when not set.
         *
         * @param key the key to get the value for.
         * @param domain the key-value pair belongs to.
         * @return the value for the key-value pair or nullopt when the key-value pair is not set.
         */
        virtual std::optional<QString> getKV(const QString& key, const Domain& domain) = 0;
        /**
         * Unset a key-value pair if it exists.
         *
         * Exit the application on error.
         *
         * @param key the key to delete the key-value pair for.
         * @param domain the Domain from which to delete the key-value pair.
         */
        virtual void deleteKV(const QString& key, const Domain& domain) = 0;

      signals:
        /**
         * Emitted when a Certificate was set or removed.
         *
         * @param cert_id the id of the Certificate that changed
         */
        void certificateChanged(StorageId cert_id);
        /**
         * Emitted when a Layout was set or removed.
         *
         * @param layout_id The id the Layout is stored under.
         * @param group The LayoutGroup this layout belongs to.
         * @param domain The Domain this layout belongs to.
         */
        void layoutChanged(StorageId layout_id, LayoutGroup group, Domain domain);
        /**
         * Emitted when a HistoricServerConnection was set or removed.
         *
         * @param historic_connection_id The id the HistoricServerConnection is stored under.
         */
        void historicConnectionChanged(StorageId historic_connection_id);
        /**
         * Emitted when a key-value pair was set or removed.
         *
         * @param key the key of the key-value pair that changed.
         * @param domain The Domain the key-value pair belongs to.
         */
        void kvChanged(QString key, Domain domain);
        // setting changed signals are sent by the SettingsManager

      private:
        /**
         * only the SettingsManager may use these
         */
        friend SettingsManager;
        /**
         * The SettingsManager needs to ensure that the type of a setting doesn't change.
         * Otherwise you could have two settings with the same name and domain for different types.
         *
         * doesn't fail when setting not set
         */
        virtual void resetSetting(const SettingKey& key) = 0;
        // Most of these could receive the same name with function overloading.
        // This is not done because doing so causes confusion when types are implicitly cast and the setting is inserted
        // into the wrong relation.
        virtual void setBooleanSetting(const SettingKey& key, bool value)                                         = 0;
        virtual void setStringSetting(const SettingKey& key, const QString& value)                                = 0;
        virtual void setIntSetting(const SettingKey& key, int value)                                              = 0;
        virtual void setDoubleSetting(const SettingKey& key, double value)                                        = 0;
        virtual void setEnumSetting(const SettingKey& key, const EnumSettingValue& value)                         = 0;
        virtual void setCertificateSetting(const SettingKey& key, StorageId cert_id)                              = 0;
        virtual void setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id)  = 0;
        virtual void setLayoutSetting(const SettingKey& key, StorageId layout_id, const LayoutGroup& group)       = 0;
        virtual std::optional<bool>                     getBoolSetting(const SettingKey& key)                     = 0;
        virtual std::optional<QString>                  getStringSetting(const SettingKey& key)                   = 0;
        virtual std::optional<int>                      getIntSetting(const SettingKey& key)                      = 0;
        virtual std::optional<double>                   getDoubleSetting(const SettingKey& key)                   = 0;
        virtual std::optional<EnumSettingValue>         getEnumSetting(const SettingKey& key)                     = 0;
        virtual std::optional<Certificate>              getCertificateSetting(const SettingKey& key)              = 0;
        virtual std::optional<HistoricServerConnection> getHistoricServerConnectionSetting(const SettingKey& key) = 0;
        virtual std::optional<Layout>                   getLayoutSetting(const SettingKey& key)                   = 0;
    };
} // namespace magnesia
