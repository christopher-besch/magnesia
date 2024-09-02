#include "SettingsManager.hpp"

#include "HistoricServerConnection.hpp"
#include "Layout.hpp"
#include "StorageManager.hpp"
#include "database_types.hpp"
#include "opcua_qt/ApplicationCertificate.hpp"
#include "settings.hpp"
#include "terminate.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QSslCertificate>
#include <QSslKey>
#include <qtmetamacros.h>

namespace magnesia {
    SettingsManager::SettingsManager(QPointer<StorageManager> storage_manager, QObject* parent)
        : QObject(parent), m_storage_manager(std::move(storage_manager)) {
        if (m_storage_manager.isNull()) {
            terminate();
        }

        connect(m_storage_manager, &StorageManager::settingDeleted, this, &SettingsManager::settingChanged);
    }

    void SettingsManager::defineSettingDomain(const Domain&                                domain,
                                              const std::vector<std::shared_ptr<Setting>>& settings) {
        if (settings.empty()) {
            m_settings.erase(domain);
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
        // using settingDeleted from the StorageManager to signal the change
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

    bool SettingsManager::setKeySetting(const SettingKey& key, StorageId key_id) {
        return setSetting<KeySetting>(key, key_id, &StorageManager::setKeySetting);
    }

    bool SettingsManager::setApplicationCertificateSetting(const SettingKey& key, StorageId cert_id) {
        return setSetting<ApplicationCertificateSetting>(key, cert_id,
                                                         &StorageManager::setApplicationCertificateSetting);
    }

    bool SettingsManager::setHistoricServerConnectionSetting(const SettingKey& key,
                                                             StorageId         historic_server_connection_id) {
        return setSetting<HistoricServerConnectionSetting>(key, historic_server_connection_id,
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
        return getSetting<DoubleSetting, double>(key, &StorageManager::getDoubleSetting);
    }

    std::optional<EnumSettingValue> SettingsManager::getEnumSetting(const SettingKey& key) const {
        return getSetting<EnumSetting, EnumSettingValue>(key, &StorageManager::getEnumSetting);
    }

    std::optional<QSslCertificate> SettingsManager::getCertificateSetting(const SettingKey& key) const {
        return getSetting<CertificateSetting, QSslCertificate>(key, &StorageManager::getCertificateSetting);
    }

    std::optional<StorageId> SettingsManager::getCertificateSettingId(const SettingKey& key) const {
        return getSetting<CertificateSetting, StorageId>(key, &StorageManager::getCertificateSettingId);
    }

    std::optional<QSslKey> SettingsManager::getKeySetting(const SettingKey& key) const {
        return getSetting<KeySetting, QSslKey>(key, &StorageManager::getKeySetting);
    }

    std::optional<StorageId> SettingsManager::getKeySettingId(const SettingKey& key) const {
        return getSetting<KeySetting, StorageId>(key, &StorageManager::getKeySettingId);
    }

    std::optional<opcua_qt::ApplicationCertificate>
    SettingsManager::getApplicationCertificateSetting(const SettingKey& key) const {
        return getSetting<ApplicationCertificateSetting, opcua_qt::ApplicationCertificate>(
            key, &StorageManager::getApplicationCertificateSetting);
    }

    std::optional<StorageId> SettingsManager::getApplicationCertificateSettingId(const SettingKey& key) const {
        return getSetting<ApplicationCertificateSetting, StorageId>(
            key, &StorageManager::getApplicationCertificateSettingId);
    }

    std::optional<HistoricServerConnection>
    SettingsManager::getHistoricServerConnectionSetting(const SettingKey& key) const {
        return getSetting<HistoricServerConnectionSetting, HistoricServerConnection>(
            key, &StorageManager::getHistoricServerConnectionSetting);
    }

    std::optional<StorageId> SettingsManager::getHistoricServerConnectionSettingId(const SettingKey& key) const {
        return getSetting<HistoricServerConnectionSetting, StorageId>(
            key, &StorageManager::getHistoricServerConnectionSettingId);
    }

    std::optional<Layout> SettingsManager::getLayoutSetting(const SettingKey& key) const {
        return getSetting<LayoutSetting, Layout>(key, &StorageManager::getLayoutSetting);
    }

    std::optional<StorageId> SettingsManager::getLayoutSettingId(const SettingKey& key) const {
        return getSetting<LayoutSetting, StorageId>(key, &StorageManager::getLayoutSettingId);
    }

    std::vector<Domain> SettingsManager::getAllDomains() const {
        auto res = std::views::transform(m_settings, &decltype(m_settings)::value_type::first);
        return {res.begin(), res.end()};
    }

    std::vector<std::shared_ptr<Setting>> SettingsManager::getSettingDefinitions(const Domain& domain) const {
        if (auto iter = m_settings.find(domain); iter != m_settings.end()) {
            return iter->second;
        }
        return {};
    }

    std::optional<std::shared_ptr<Setting>> SettingsManager::findSettingDefinition(const SettingKey& key) const {
        auto settings_in_domain = getSettingDefinitions(key.domain);
        if (auto iter = std::ranges::find(settings_in_domain, key.name, &Setting::getName);
            iter != settings_in_domain.end()) {
            return *iter;
        }
        return std::nullopt;
    }

    template<typename SettingsType, typename ValueType>
    SettingsType* SettingsManager::validate(const SettingKey& key, const ValueType& value) const {
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
