#pragma once

#include "StorageManager.hpp"
#include "database_types.hpp"
#include "settings.hpp"

#include <optional>

#include <QList>
#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {

    /*
     * Settings layer on top of StorageManager.
     */
    class SettingsManager : QObject {
        Q_OBJECT

      public:
        /*
         * @param storage_manager The StorageManager to be used by the SettingsManager.
         *     The StorageManager must live at least as long as the Settingsmanager.
         */
        explicit SettingsManager(StorageManager* storage_manager);
        /*
         * Define or redefine all settings for a domain.
         *
         * @param domain The Domain to (re)define.
         * @param settings The settings to belong to that Domain.
         */
        void defineSettingDomain(const Domain& domain, const QList<QSharedPointer<Setting>>& settings);

        /*
         * Reset a setting to it's default value.
         *
         * Fail when the setting can't be found.
         *
         * @param key The SettingKey of the setting to be reset.
         *
         * @return false on failure.
         */
        bool resetSetting(const SettingKey& key);

        // Most of these could receive the same name with function overloading.
        // This is not done because doing so causes confusion when types are implicitly cast and the setting is inserted
        // into the wrong table.
        // In a perfect world the SettingsManager would deduce the type from the SettingKey but that can't be done at
        // compile time.

        /*
         * Change a BooleanSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param value the new value.
         *
         * @return false on failure.
         */
        bool setBooleanSetting(const SettingKey& key, const bool& value);
        /*
         * Change a StringSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param value the new value.
         *
         * @return false on failure.
         */
        bool setStringSetting(const SettingKey& key, const QString& value);
        /*
         * Change an IntSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param value the new value.
         *
         * @return false on failure.
         */
        bool setIntSetting(const SettingKey& key, const int& value);
        /*
         * Change a setFloatSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param value the new value.
         *
         * @return false on failure.
         */
        bool setFloatSetting(const SettingKey& key, const float& value);
        /*
         * Change an EnumSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param value the new value.
         *
         * @return false on failure.
         */
        bool setEnumSetting(const SettingKey& key, const EnumSettingValue& value);
        /*
         * Change a CertificateSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param cert_id the id of the Certificate.
         *
         * @return false on failure.
         */
        bool setCertificateSetting(const SettingKey& key, StorageId cert_id);
        /*
         * Change a HistoricServerConnectionSetting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param historic_connection_id The HistoricServerConnection's id.
         *
         * @return false on failure.
         */
        bool setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id);
        /*
         * Change a LayoutSetting.
         *
         * The layout must belong to the Domain of the setting and the Group defined in the setting.
         *
         * Fail when the setting can't be found or doesn't fulfill the setting's type.
         *
         * @param key The SettingKey of the setting to be set.
         * @param layout_id the id of the Layout.
         *
         * @return false on failure.
         */
        bool setLayoutSetting(const SettingKey& key, StorageId layout_id);

        /*
         * Get a BooleanSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<bool> getBoolSetting(const SettingKey& key);
        /*
         * Get a StringSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<QString> getStringSetting(const SettingKey& key);
        /*
         * Get an IntSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<int> getIntSetting(const SettingKey& key);
        /*
         * Get a FloatSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<float> getFloatSetting(const SettingKey& key);
        /*
         * Get an EnumSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<EnumSettingValue> getEnumSetting(const SettingKey& key);
        /*
         * Get a CertificateSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<Certificate> getCertificateSetting(const SettingKey& key);
        /*
         * Get a HistoricServerConnectionSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<HistoricServerConnection> getHistoricServerConnectionSetting(const SettingKey& key);
        /*
         * Get a LayoutSetting.
         *
         * @param key The SettingKey of the setting to be get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined
         */
        std::optional<Layout> getLayoutSetting(const SettingKey& key);

        /**
         * Get all Domains in correct order.
         *
         * @return the ordered list of domains.
         */
        QList<Domain> getAllDomains();
        /**
         * Get all settings in correct order for a Domain.
         *
         * When the domain is not defined return an empty list.
         *
         * @param domain the domain to get the settings for.
         *
         * @return the ordered list of settings.
         */
        QList<QSharedPointer<Setting>> getSettingDefinitionsInDomain(const Domain& domain);

      private:
        /**
         * Delete all Setting definitions for a domain.
         *
         * This doesn't touch the StorageManager.
         *
         * @param domain The domain to be deleted.
         */
        void deleteDomain(const Domain& domain);

        std::optional<QSharedPointer<Setting>> getSettingDefinition(const SettingKey& key);

        template<typename S, typename V>
        S* validate(const SettingKey& key, V value) {
            auto setting = getSettingDefinition(key);
            if (setting == std::nullopt) {
                return nullptr;
            }
            auto* specific_setting = dynamic_cast<S*>(setting.value().get());
            if (specific_setting == nullptr) {
                return nullptr;
            }
            if (!specific_setting->isValid(value)) {
                return nullptr;
            }
            return specific_setting;
        }

      signals:
        /**
         * Qt signal called when a setting was set or reset.
         *
         * @param key The key of the Setting that was (re)set.
         */
        void settingChanged(const SettingKey& key);
        /**
         * Qt signal called when a setting domain was (re)defined.
         *
         * @param domain The Domain that was (re)defined.
         */
        void settingDomainDefined(const Domain& domain);

      private:
        QMap<Domain, QList<QSharedPointer<Setting>>> m_settings;
        StorageManager*                              m_storage_manager{nullptr};
    };

} // namespace magnesia
