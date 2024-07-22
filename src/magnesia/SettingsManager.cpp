#include "SettingsManager.hpp"

#include "StorageManager.hpp"
#include "database_types.hpp"
#include "settings.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QString>
#include <qtmetamacros.h>

namespace magnesia {
    SettingsManager::SettingsManager(QPointer<StorageManager> storage_manager, QObject* parent)
        : QObject(parent), m_storage_manager(std::move(storage_manager)) {
        if (m_storage_manager.isNull()) {
            terminate();
        }
    }

    void SettingsManager::defineSettingDomain(const Domain& domain, const QList<QSharedPointer<Setting>>& settings) {
        if (settings.isEmpty()) {
            m_settings.remove(domain);
        } else {
            m_settings[domain] = settings;
        }
        Q_EMIT settingDomainDefined(domain);
    }

    bool SettingsManager::resetSetting(const SettingKey& key) {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return false;
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        m_storage_manager->resetSetting(key);
        Q_EMIT settingChanged(key);
        return true;
    }

    template<typename SettingType>
    bool SettingsManager::setSetting(const SettingKey& key, auto&& value, auto&& setter) {
        auto* setting = validate<SettingType>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        std::invoke(std::forward<decltype(setter)>(setter), storage_manager, key, std::forward<decltype(value)>(value));
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setBooleanSetting(const SettingKey& key, bool value) {
        return setSetting<BooleanSetting>(key, value, &StorageManager::setBooleanSetting);
    }

    bool SettingsManager::setStringSetting(const SettingKey& key, const QString& value) {
        return setSetting<StringSetting>(key, value, &StorageManager::setStringSetting);
    }

    bool SettingsManager::setIntSetting(const SettingKey& key, std::int64_t value) {
        return setSetting<IntSetting>(key, value, &StorageManager::setIntSetting);
    }

    bool SettingsManager::setDoubleSetting(const SettingKey& key, double value) {
        return setSetting<DoubleSetting>(key, value, &StorageManager::setDoubleSetting);
    }

    bool SettingsManager::setEnumSetting(const SettingKey& key, const EnumSettingValue& value) {
        return setSetting<EnumSetting>(key, value, &StorageManager::setEnumSetting);
    }

    bool SettingsManager::setCertificateSetting(const SettingKey& key, StorageId cert_id) {
        return setSetting<CertificateSetting>(key, cert_id, &StorageManager::setCertificateSetting);
    }

    bool SettingsManager::setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id) {
        return setSetting<HistoricServerConnectionSetting>(key, historic_connection_id,
                                                           &StorageManager::setHistoricServerConnectionSetting);
    }

    bool SettingsManager::setLayoutSetting(const SettingKey& key, StorageId layout_id) {
        auto* setting = validate<LayoutSetting>(key, layout_id);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setLayoutSetting(key, layout_id, setting->getGroup());
        Q_EMIT settingChanged(key);
        return true;
    }

    template<typename SettingType, typename T>
    std::optional<T> SettingsManager::getSetting(const SettingKey& key, auto&& getter) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return std::nullopt;
        }
        auto* setting_type = dynamic_cast<SettingType*>(setting.value().get());
        if (setting_type == nullptr) {
            return std::nullopt;
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        auto res = std::invoke(std::forward<decltype(getter)>(getter), m_storage_manager, key);
        if constexpr (requires { setting_type->getDefault(); }) {
            return res.value_or(setting_type->getDefault());
        } else {
            return res;
        }
    }

    std::optional<bool> SettingsManager::getBoolSetting(const SettingKey& key) const {
        return getSetting<BooleanSetting, bool>(key, &StorageManager::getBoolSetting);
    }

    std::optional<QString> SettingsManager::getStringSetting(const SettingKey& key) const {
        return getSetting<StringSetting, QString>(key, &StorageManager::getStringSetting);
    }

    std::optional<std::int64_t> SettingsManager::getIntSetting(const SettingKey& key) const {
        return getSetting<IntSetting, std::int64_t>(key, &StorageManager::getIntSetting);
    }

    std::optional<double> SettingsManager::getDoubleSetting(const SettingKey& key) const {
        return getSetting<DoubleSetting, std::int64_t>(key, &StorageManager::getDoubleSetting);
    }

    std::optional<EnumSettingValue> SettingsManager::getEnumSetting(const SettingKey& key) const {
        return getSetting<EnumSetting, EnumSettingValue>(key, &StorageManager::getEnumSetting);
    }

    std::optional<Certificate> SettingsManager::getCertificateSetting(const SettingKey& key) const {
        return getSetting<CertificateSetting, Certificate>(key, &StorageManager::getCertificateSetting);
    }

    std::optional<HistoricServerConnection>
    SettingsManager::getHistoricServerConnectionSetting(const SettingKey& key) const {
        return getSetting<HistoricServerConnectionSetting, HistoricServerConnection>(
            key, &StorageManager::getHistoricServerConnectionSetting);
    }

    std::optional<Layout> SettingsManager::getLayoutSetting(const SettingKey& key) const {
        return getSetting<LayoutSetting, Layout>(key, &StorageManager::getLayoutSetting);
    }

    QList<Domain> SettingsManager::getAllDomains() const {
        return m_settings.keys();
    }

    QList<QSharedPointer<Setting>> SettingsManager::getSettingDefinitions(const Domain& domain) const {
        // This returns an empty list if domain not defined.
        return m_settings[domain];
    }

    std::optional<QSharedPointer<Setting>> SettingsManager::findSettingDefinition(const SettingKey& key) const {
        auto settings_in_domain = getSettingDefinitions(key.domain);
        for (const auto& setting : settings_in_domain) {
            if (setting->getName() == key.name) {
                return setting;
            }
        }
        return std::nullopt;
    }

    template<typename SettingsType, typename ValueType>
    SettingsType* SettingsManager::validate(const SettingKey& key, ValueType value) const {
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
} // namespace magnesia
