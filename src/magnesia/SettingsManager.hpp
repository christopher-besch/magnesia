#pragma once

#include "StorageManager.hpp"
#include "database_types.hpp"
#include "settings.hpp"

#include <cstdint>
#include <optional>

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {
    /*
     * Settings layer on top of StorageManager.
     */
    class SettingsManager : public QObject {
        Q_OBJECT

      public:
        /*
         * @param storage_manager The StorageManager to be used as underlying storage.
         *     It must live for at least the lifetime of this object.
         */
        explicit SettingsManager(QPointer<StorageManager> storage_manager, QObject* parent = nullptr);
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
         * @param key The SettingKey of the setting to reset.
         *
         * @return false on failure, true otherwise.
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
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param value the new value.
         *
         * @return false on failure, true otherwise.
         */
        bool setBooleanSetting(const SettingKey& key, bool value);
        /*
         * Change a StringSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param value the new value.
         *
         * @return false on failure, true otherwise.
         */
        bool setStringSetting(const SettingKey& key, const QString& value);
        /*
         * Change an IntSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param value the new value.
         *
         * @return false on failure, true otherwise.
         */
        bool setIntSetting(const SettingKey& key, std::int64_t value);
        /*
         * Change a setDoubleSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param value the new value.
         *
         * @return false on failure, true otherwise.
         */
        bool setDoubleSetting(const SettingKey& key, double value);
        /*
         * Change an EnumSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param value the new value.
         *
         * @return false on failure, true otherwise.
         */
        bool setEnumSetting(const SettingKey& key, const EnumSettingValue& value);
        /*
         * Change a CertificateSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param cert_id the id of the Certificate.
         *
         * @return false on failure, true otherwise.
         */
        bool setCertificateSetting(const SettingKey& key, StorageId cert_id);
        /*
         * Change a HistoricServerConnectionSetting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param historic_connection_id The HistoricServerConnection's id.
         *
         * @return false on failure, true otherwise.
         */
        bool setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id);
        /*
         * Change a LayoutSetting.
         *
         * The layout must belong to the Domain of the setting and the Group defined in the setting.
         *
         * Fail when the setting can't be found or value is invalid.
         *
         * @param key The SettingKey of the setting to set.
         * @param layout_id the id of the Layout.
         *
         * @return false on failure, true otherwise.
         */
        bool setLayoutSetting(const SettingKey& key, StorageId layout_id);

        /*
         * Get a BooleanSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<bool> getBoolSetting(const SettingKey& key) const;
        /*
         * Get a StringSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<QString> getStringSetting(const SettingKey& key) const;
        /*
         * Get an IntSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<std::int64_t> getIntSetting(const SettingKey& key) const;
        /*
         * Get a DoubleSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<double> getDoubleSetting(const SettingKey& key) const;
        /*
         * Get an EnumSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<EnumSettingValue> getEnumSetting(const SettingKey& key) const;
        /*
         * Get a CertificateSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<Certificate> getCertificateSetting(const SettingKey& key) const;
        /*
         * Get a HistoricServerConnectionSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<HistoricServerConnection>
        getHistoricServerConnectionSetting(const SettingKey& key) const;
        /*
         * Get a LayoutSetting.
         *
         * @param key The SettingKey of the setting to get.
         *
         * @return the Setting's value or its default value when not set or nullopt when the setting is not defined.
         */
        [[nodiscard]] std::optional<Layout> getLayoutSetting(const SettingKey& key) const;

        /**
         * Get all Domains in the order they're defined in.
         *
         * @return the ordered list of domains.
         */
        [[nodiscard]] QList<Domain> getAllDomains() const;
        /**
         * Get all settings for a domain in the order they're defined in.
         *
         * When the domain is not defined return an empty list.
         *
         * @param domain the domain to get the settings for.
         *
         * @return the ordered list of settings.
         */
        [[nodiscard]] QList<QSharedPointer<Setting>> getSettingDefinitions(const Domain& domain) const;

      signals:
        /**
         * Emitted when a setting was set or reset.
         *
         * @param key The key of the Setting that was (re)set.
         */
        void settingChanged(SettingKey key);
        /**
         * Emitted when a setting domain was (re)defined.
         *
         * @param domain The Domain that was (re)defined.
         */
        void settingDomainDefined(Domain domain);

      private:
        [[nodiscard]] std::optional<QSharedPointer<Setting>> findSettingDefinition(const SettingKey& key) const;

        /**
         * Check if a value can be used for a specific setting.
         *
         * @param key The key of the setting the value is for.
         * @param value The value to check.
         * @return nullptr when the value is not valid, return the setting definition otherwise.
         */
        template<typename SettingsType, typename ValueType>
        SettingsType* validate(const SettingKey& key, ValueType value) const {
            auto setting = findSettingDefinition(key);
            if (setting == std::nullopt) {
                return nullptr;
            }
            auto* specific_setting = dynamic_cast<SettingsType*>(setting.value().get());
            if (specific_setting == nullptr) {
                return nullptr;
            }
            if (!specific_setting->isValid(value)) {
                return nullptr;
            }
            return specific_setting;
        }

      private:
        QMap<Domain, QList<QSharedPointer<Setting>>> m_settings;
        QPointer<StorageManager>                     m_storage_manager;
    };
} // namespace magnesia
