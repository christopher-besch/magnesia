#include "settings.hpp"

#include "database_types.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <utility>

#include <QSet>

namespace magnesia {
    QString Setting::getName() const {
        return m_name;
    }

    QString Setting::getHumanReadableName() const {
        return m_human_readable_name;
    }

    QString Setting::getDescription() const {
        return m_description;
    }

    Setting::Setting(QString name, QString human_readable_name, QString description)
        : m_name{std::move(name)}, m_human_readable_name{std::move(human_readable_name)},
          m_description{std::move(description)} {};

    bool BooleanSetting::getDefault() const {
        return m_default_value;
    }

    bool BooleanSetting::isValid(bool /*value*/) {
        return true;
    }

    BooleanSetting::BooleanSetting(QString name, QString human_readable_name, QString description, bool default_value)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_default_value{default_value} {}

    QString StringSetting::getDefault() const {
        return m_default_value;
    }

    bool StringSetting::isValid(const QString& /*value*/) {
        return true;
    }

    StringSetting::StringSetting(QString name, QString human_readable_name, QString description, QString default_value)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_default_value{std::move(default_value)} {}

    std::int64_t IntSetting::getDefault() const {
        return m_default_value;
    }

    std::int64_t IntSetting::getMin() const {
        return m_min;
    }

    std::int64_t IntSetting::getMax() const {
        return m_max;
    }

    bool IntSetting::isValid(std::int64_t value) const {
        return value >= m_min && value <= m_max;
    }

    IntSetting::IntSetting(QString name, QString human_readable_name, QString description, std::int64_t default_value,
                           std::int64_t min, std::int64_t max)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_default_value{default_value}, m_min{min}, m_max{max} {
        if (!isValid(m_default_value)) {
            terminate();
        }
    }

    double DoubleSetting::getDefault() const {
        return m_default_value;
    }

    double DoubleSetting::getMin() const {
        return m_min;
    }

    double DoubleSetting::getMax() const {
        return m_max;
    }

    bool DoubleSetting::isValid(double value) const {
        return value >= m_min && value <= m_max;
    }

    DoubleSetting::DoubleSetting(QString name, QString human_readable_name, QString description, double default_value,
                                 double min, double max)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_default_value{default_value}, m_min{min}, m_max{max} {
        if (!isValid(m_default_value)) {
            terminate();
        }
    }

    EnumSettingValue EnumSetting::getDefault() const {
        return m_default_value;
    }

    const QSet<EnumSettingValue>& EnumSetting::getPossibleValues() const {
        return m_possible_values;
    }

    bool EnumSetting::isValid(const EnumSettingValue& value) const {
        return m_possible_values.contains(value);
    }

    EnumSetting::EnumSetting(QString name, QString human_readable_name, QString description,
                             EnumSettingValue default_value, QSet<EnumSettingValue> possible_values)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_default_value{std::move(default_value)}, m_possible_values{std::move(possible_values)} {
        if (!isValid(m_default_value)) {
            terminate();
        }
    }

    bool HistoricServerConnectionSetting::isValid(StorageId /*value*/) {
        return true;
    }

    HistoricServerConnectionSetting::HistoricServerConnectionSetting(QString name, QString human_readable_name,
                                                                     QString description)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}

    bool CertificateSetting::isValid(StorageId /*value*/) {
        return true;
    }

    CertificateSetting::CertificateSetting(QString name, QString human_readable_name, QString description)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}

    bool ApplicationCertificateSetting::isValid(StorageId /*value*/) {
        return true;
    }

    ApplicationCertificateSetting::ApplicationCertificateSetting(QString name, QString human_readable_name,
                                                                 QString description)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}

    bool KeySetting::isValid(StorageId /*value*/) {
        return true;
    }

    KeySetting::KeySetting(QString name, QString human_readable_name, QString description)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}

    LayoutGroup LayoutSetting::getGroup() const {
        return m_layout_group;
    }

    bool LayoutSetting::isValid(StorageId /*value*/) {
        return true;
    }

    LayoutSetting::LayoutSetting(QString name, QString human_readable_name, QString description,
                                 LayoutGroup layout_group)
        : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
          m_layout_group{std::move(layout_group)} {}
} // namespace magnesia
