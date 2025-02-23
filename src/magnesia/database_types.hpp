#pragma once

#include <QString>

namespace magnesia {
    /**
     * The type of an enum value for the EnumSetting.
     */
    using EnumSettingValue = QString;
    /**
     * Many database objects use this id to be identified.
     *
     * SQLite supports 64bit rowids.
     *
     * This needs to be convertible into QVariant
     */
    using StorageId = unsigned long long;
    /**
     * Settings, layouts and key-value pairs are separated into domains.
     * Every activity shall have their own domain(s).
     * These are used to e.g. group settings together in the SettingsActivity.
     */
    using Domain = QString;
    /**
     * The name of a setting in the database.
     */
    using SettingName = QString;
    /**
     * Layouts are grouped by Group to allow activities to have
     */
    using LayoutGroup = QString;

    /**
     * @class SettingKey
     * @brief The identifier for a setting.
     */
    struct SettingKey {
        /// Name of the Setting.
        SettingName name;
        /// Domain containing the setting.
        Domain domain;
    };

    inline bool operator<(const SettingKey& lhs, const SettingKey& rhs) {
        if (lhs.domain != rhs.domain) {
            return lhs.domain < rhs.domain;
        }
        return lhs.name < rhs.name;
    }
} // namespace magnesia
