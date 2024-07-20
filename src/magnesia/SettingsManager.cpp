#include "SettingsManager.hpp"

#include "StorageManager.hpp"
#include "database_types.hpp"
#include "settings.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <optional>
#include <utility>

#include <QList>
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

    bool SettingsManager::setBooleanSetting(const SettingKey& key, bool value) {
        auto* setting = validate<BooleanSetting>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setBooleanSetting(key, value);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setStringSetting(const SettingKey& key, const QString& value) {
        auto* setting = validate<StringSetting>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setStringSetting(key, value);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setIntSetting(const SettingKey& key, std::int64_t value) {
        auto* setting = validate<IntSetting>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setIntSetting(key, value);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setDoubleSetting(const SettingKey& key, double value) {
        auto* setting = validate<DoubleSetting>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setDoubleSetting(key, value);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setEnumSetting(const SettingKey& key, const EnumSettingValue& value) {
        auto* setting = validate<EnumSetting>(key, value);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setEnumSetting(key, value);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setCertificateSetting(const SettingKey& key, StorageId cert_id) {
        auto* setting = validate<CertificateSetting>(key, cert_id);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setCertificateSetting(key, cert_id);
        Q_EMIT settingChanged(key);
        return true;
    }

    bool SettingsManager::setHistoricServerConnectionSetting(const SettingKey& key, StorageId historic_connection_id) {
        auto* setting = validate<HistoricServerConnectionSetting>(key, historic_connection_id);
        if (setting == nullptr) {
            return false;
        }
        auto* storage_manager = m_storage_manager.get();
        if (storage_manager == nullptr) {
            terminate();
        }
        storage_manager->setHistoricServerConnectionSetting(key, historic_connection_id);
        Q_EMIT settingChanged(key);
        return true;
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

    std::optional<bool> SettingsManager::getBoolSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* bool_setting = dynamic_cast<BooleanSetting*>(setting.value().get());
        if (bool_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        return m_storage_manager->getBoolSetting(key).value_or(bool_setting->getDefault());
    }

    std::optional<QString> SettingsManager::getStringSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* string_setting = dynamic_cast<StringSetting*>(setting.value().get());
        if (string_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        return m_storage_manager->getStringSetting(key).value_or(string_setting->getDefault());
    }

    std::optional<std::int64_t> SettingsManager::getIntSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* int_setting = dynamic_cast<IntSetting*>(setting.value().get());
        if (int_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        return m_storage_manager->getIntSetting(key).value_or(int_setting->getDefault());
    }

    std::optional<double> SettingsManager::getDoubleSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* double_setting = dynamic_cast<DoubleSetting*>(setting.value().get());
        if (double_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        return m_storage_manager->getDoubleSetting(key).value_or(double_setting->getDefault());
    }

    std::optional<EnumSettingValue> SettingsManager::getEnumSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* enum_setting = dynamic_cast<EnumSetting*>(setting.value().get());
        if (enum_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        return m_storage_manager->getEnumSetting(key).value_or(enum_setting->getDefault());
    }

    std::optional<Certificate> SettingsManager::getCertificateSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* cert_setting = dynamic_cast<CertificateSetting*>(setting.value().get());
        if (cert_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        // Leave nullopt when not set.
        return m_storage_manager->getCertificateSetting(key);
    }

    std::optional<HistoricServerConnection>
    SettingsManager::getHistoricServerConnectionSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* server_con_setting = dynamic_cast<HistoricServerConnectionSetting*>(setting.value().get());
        if (server_con_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        // Leave nullopt when not set.
        return m_storage_manager->getHistoricServerConnectionSetting(key);
    }

    std::optional<Layout> SettingsManager::getLayoutSetting(const SettingKey& key) const {
        auto setting = findSettingDefinition(key);
        if (setting == std::nullopt) {
            return {};
        }
        auto* layout_setting = dynamic_cast<LayoutSetting*>(setting.value().get());
        if (layout_setting == nullptr) {
            return {};
        }
        if (m_storage_manager.isNull()) {
            terminate();
        }
        // Leave nullopt when not set.
        return m_storage_manager->getLayoutSetting(key);
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
} // namespace magnesia
