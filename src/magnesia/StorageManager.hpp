#pragma once

#include "HistoricServerConnection.hpp"
#include "Layout.hpp"
#include "database_types.hpp"
#include "opcua_qt/ApplicationCertificate.hpp"

#include <cstdint>
#include <optional>

#include <QList>
#include <QObject>
#include <QSslCertificate>
#include <QSslKey>
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
         * Store an X.509 certificate in the database.
         *
         * Exit the application on error.
         *
         * @param cert the certificate to store.
         * @return the new database id of the certificate.
         */
        virtual StorageId storeCertificate(const QSslCertificate& cert) = 0;
        /**
         * Store an X.509 key in the database.
         *
         * Exit the application on error.
         *
         * @param key the key to store.
         * @return the new database id of the key.
         */
        virtual StorageId storeKey(const QSslKey& key) = 0;
        /**
         * Store an X.509 certificate key pair in the database.
         *
         * Exit the application on error.
         *
         * @param cert the certificate key pair to store.
         * @return the new database id of the certificate key pair.
         */
        virtual StorageId storeApplicationCertificate(const opcua_qt::ApplicationCertificate& cert) = 0;
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
         * Retrieve an X.509 certificate from the database.
         *
         * Exit the application on error.
         *
         * @param cert_id The id the certificate is stored under.
         * @return the certificate or nullopt when not found.
         */
        [[nodiscard]] virtual std::optional<QSslCertificate> getCertificate(StorageId cert_id) const = 0;
        /**
         * Retrieve an X.509 key from the database.
         *
         * Exit the application on error.
         *
         * @param key_id The id the key is stored under.
         * @return the key or nullopt when not found.
         */
        [[nodiscard]] virtual std::optional<QSslKey> getKey(StorageId key_id) const = 0;
        /**
         * Retrieve an X.509 certificate key pair from the database.
         *
         * Exit the application on error.
         *
         * @param cert_id The id the certificate key pair is stored under.
         * @return the ApplicationCertificate or nullopt when not found.
         */
        [[nodiscard]] virtual std::optional<opcua_qt::ApplicationCertificate>
        getApplicationCertificate(StorageId cert_id) const = 0;
        /**
         * Retrieve a HistoricServerConnection from the database.
         *
         * Exit the application on error.
         *
         * @param historic_server_connection_id The id the HistoricServerConnection is stored under.
         * @return the HistoricServerConnection or nullopt when not found.
         */
        [[nodiscard]] virtual std::optional<HistoricServerConnection>
        getHistoricServerConnection(StorageId historic_server_connection_id) const = 0;
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
        [[nodiscard]] virtual std::optional<Layout> getLayout(StorageId layout_id, const LayoutGroup& group,
                                                              const Domain& domain) const = 0;

        /**
         * Retrieve all X.509 certificates from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all certificates.
         */
        [[nodiscard]] virtual QList<QSslCertificate> getAllCertificates() const = 0;
        /**
         * Retrieve all X.509 keys from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all keys.
         */
        [[nodiscard]] virtual QList<QSslKey> getAllKeys() const = 0;
        /**
         * Retrieve all X.509 certificate key pairs from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all certificate key pairs.
         */
        [[nodiscard]] virtual QList<opcua_qt::ApplicationCertificate> getAllApplicationCertificates() const = 0;
        /**
         * Retrieve all HistoricServerConnections from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all HistoricServerConnections.
         */
        [[nodiscard]] virtual QList<HistoricServerConnection> getAllHistoricServerConnections() const = 0;
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
        [[nodiscard]] virtual QList<Layout> getAllLayouts(const LayoutGroup& group, const Domain& domain) const = 0;

        /**
         * Retrieve all X.509 certificate IDs from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all certificate IDs.
         */
        [[nodiscard]] virtual QList<StorageId> getAllCertificateIds() const = 0;
        /**
         * Retrieve all X.509 key IDs from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all key IDs.
         */
        [[nodiscard]] virtual QList<StorageId> getAllKeyIds() const = 0;
        /**
         * Retrieve all X.509 certificate key pair IDs from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all certificate key pair IDs.
         */
        [[nodiscard]] virtual QList<StorageId> getAllApplicationCertificateIds() const = 0;
        /**
         * Retrieve all HistoricServerConnection IDs from the database.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @return a list of all HistoricServerConnections IDs.
         */
        [[nodiscard]] virtual QList<StorageId> getAllHistoricServerConnectionIds() const = 0;
        /**
         * Retrieve all Layout IDs from the database for a specified group and domain.
         *
         * This can be used to populate drop down menus in the SettingsActivity.
         *
         * Exit the application on error.
         *
         * @param group The LayoutGroup the layouts belong to.
         * @param domain The Domain the layouts belong to.
         * @return a list of all Layout IDs with the specified group and domain.
         */
        [[nodiscard]] virtual QList<StorageId> getAllLayoutIds(const LayoutGroup& group,
                                                               const Domain&      domain) const = 0;

        /**
         * Delete the X.509 certificate with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param cert_id The id of the certificate.
         */
        virtual void deleteCertificate(StorageId cert_id) = 0;
        /**
         * Delete the X.509 key with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param key_id The id of the key.
         */
        virtual void deleteKey(StorageId key_id) = 0;
        /**
         * Delete the X.509 certificate key pair with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param cert_id The id of the certificate key pair.
         */
        virtual void deleteApplicationCertificate(StorageId cert_id) = 0;
        /**
         * Delete the HistoricServerConnection with the specified id if it exists.
         *
         * Exit the application on error.
         *
         * @param historic_server_connection_id The id of the HistoricServerConnection.
         */
        virtual void deleteHistoricServerConnection(StorageId historic_server_connection_id) = 0;
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
        [[nodiscard]] virtual std::optional<QString> getKV(const QString& key, const Domain& domain) const = 0;
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
         * Emitted when an X.509 certificate was set or removed.
         *
         * @param cert_id the id of the certificate that changed
         */
        void certificateChanged(StorageId cert_id);
        /**
         * Emitted when an X.509 key was set or removed.
         *
         * @param key_id the id of the key that changed
         */
        void keyChanged(StorageId key_id);
        /**
         * Emitted when an X.509 certificate key pair was set or removed.
         *
         * @param cert_id the id of the certificate key pair that changed
         */
        void applicationCertificateChanged(StorageId cert_id);
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
         * @param historic_server_connection_id The id the HistoricServerConnection is stored under.
         */
        void historicServerConnectionChanged(StorageId historic_server_connection_id);
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
        virtual void setBooleanSetting(const SettingKey& key, bool value)                                          = 0;
        virtual void setStringSetting(const SettingKey& key, const QString& value)                                 = 0;
        virtual void setIntSetting(const SettingKey& key, std::int64_t value)                                      = 0;
        virtual void setDoubleSetting(const SettingKey& key, double value)                                         = 0;
        virtual void setEnumSetting(const SettingKey& key, const EnumSettingValue& value)                          = 0;
        virtual void setCertificateSetting(const SettingKey& key, StorageId cert_id)                               = 0;
        virtual void setKeySetting(const SettingKey& key, StorageId key_id)                                        = 0;
        virtual void setApplicationCertificateSetting(const SettingKey& key, StorageId cert_id)                    = 0;
        virtual void setHistoricServerConnectionSetting(const SettingKey& key,
                                                        StorageId         historic_server_connection_id)                   = 0;
        virtual void setLayoutSetting(const SettingKey& key, StorageId layout_id, const LayoutGroup& group)        = 0;
        [[nodiscard]] virtual std::optional<bool>             getBoolSetting(const SettingKey& key) const          = 0;
        [[nodiscard]] virtual std::optional<QString>          getStringSetting(const SettingKey& key) const        = 0;
        [[nodiscard]] virtual std::optional<std::int64_t>     getIntSetting(const SettingKey& key) const           = 0;
        [[nodiscard]] virtual std::optional<double>           getDoubleSetting(const SettingKey& key) const        = 0;
        [[nodiscard]] virtual std::optional<EnumSettingValue> getEnumSetting(const SettingKey& key) const          = 0;
        [[nodiscard]] virtual std::optional<QSslCertificate>  getCertificateSetting(const SettingKey& key) const   = 0;
        [[nodiscard]] virtual std::optional<StorageId>        getCertificateSettingId(const SettingKey& key) const = 0;
        [[nodiscard]] virtual std::optional<QSslKey>          getKeySetting(const SettingKey& key) const           = 0;
        [[nodiscard]] virtual std::optional<StorageId>        getKeySettingId(const SettingKey& key) const         = 0;
        [[nodiscard]] virtual std::optional<opcua_qt::ApplicationCertificate>
        getApplicationCertificateSetting(const SettingKey& key) const = 0;
        [[nodiscard]] virtual std::optional<StorageId>
        getApplicationCertificateSettingId(const SettingKey& key) const = 0;
        [[nodiscard]] virtual std::optional<HistoricServerConnection>
        getHistoricServerConnectionSetting(const SettingKey& key) const = 0;
        [[nodiscard]] virtual std::optional<StorageId>
        getHistoricServerConnectionSettingId(const SettingKey& key) const                              = 0;
        [[nodiscard]] virtual std::optional<Layout>    getLayoutSetting(const SettingKey& key) const   = 0;
        [[nodiscard]] virtual std::optional<StorageId> getLayoutSettingId(const SettingKey& key) const = 0;

      signals:
        /**
         * Emitted when a setting was deleted (i.e., reset).
         *
         * @param key the key of the setting that was deleted.
         */
        void settingDeleted(SettingKey key);
    };
} // namespace magnesia
