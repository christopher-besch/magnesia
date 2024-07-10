#pragma once

#include "database_types.hpp"
#include "terminate.hpp"

#include <utility>

#include <QSet>
#include <QString>

namespace magnesia {
    /**
     * Base class for all Settings definitions.
     *
     * The SettingsManager needs to know what settings to manage.
     * These are defined using subclasses of this base class.
     *
     * Therefore, these classes don't contain the currently set value for any setting.
     * They only contain data that shouldn't be changable by the user.
     */
    class Setting {
      public:
        Setting(const Setting&)            = default;
        Setting(Setting&&)                 = default;
        Setting& operator=(const Setting&) = default;
        Setting& operator=(Setting&&)      = default;
        virtual ~Setting()                 = default;

        QString getName() {
            return m_name;
        }

        QString getHumanReadableName() {
            return m_human_readable_name;
        }

        QString getDescription() {
            return m_description;
        }

      protected:
        Setting(QString name, QString human_readable_name, QString description)
            : m_name{std::move(name)}, m_human_readable_name{std::move(human_readable_name)},
              m_description{std::move(description)} {};

      private:
        /**
         * The name and domain are needed as primary keys for the database.
         * As only entire domains of settings can get created, the domain is not needed here.
         */
        QString m_name;
        /**
         * This attribute allows changing the display name inside the Settings Activity without having to alter the
         * database.
         */
        QString m_human_readable_name;
        /**
         * The description of this setting to be shown in the Settings Activity.
         */
        QString m_description;
    };

    /**
     * A setting of type bool.
     */
    class BooleanSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] bool getDefault() const {
            return m_default_value;
        }

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] static bool isValid(bool /*value*/) {
            return true;
        }

        BooleanSetting(QString name, QString human_readable_name, QString description, bool default_value)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_default_value{default_value} {}

      private:
        bool m_default_value;
    };

    /**
     * A setting of type QString.
     */
    class StringSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] QString getDefault() const {
            return m_default_value;
        }

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] static bool isValid(const QString& /*value*/) {
            return true;
        }

        StringSetting(QString name, QString human_readable_name, QString description, QString default_value)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_default_value{std::move(default_value)} {}

      private:
        QString m_default_value;
    };

    /**
     * A setting of type int in [min, max].
     */
    class IntSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] int getDefault() const {
            return m_default_value;
        }

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] bool isValid(int value) const {
            return value >= m_min && value <= m_max;
        }

        IntSetting(QString name, QString human_readable_name, QString description, int default_value, int min, int max)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_default_value{default_value}, m_min{min}, m_max{max} {
            if (!isValid(m_default_value)) {
                terminate();
            }
        }

      private:
        int m_default_value;
        int m_min;
        int m_max;
    };

    /**
     * A setting of type float in [min, max].
     */
    class FloatSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] float getDefault() const {
            return m_default_value;
        }

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] bool isValid(float value) const {
            return value >= m_min && value <= m_max;
        }

        FloatSetting(QString name, QString human_readable_name, QString description, float default_value, float min,
                     float max)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_default_value{default_value}, m_min{min}, m_max{max} {
            if (!isValid(m_default_value)) {
                terminate();
            }
        }

      private:
        float m_default_value;
        float m_min;
        float m_max;
    };

    /**
     * A setting of type enum.
     * The setting's value must be one of the possible_values.
     */
    class EnumSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] EnumSettingValue getDefault() const {
            return m_default_value;
        }

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] bool isValid(const EnumSettingValue& value) const {
            return m_possible_values.contains(value);
        }

        EnumSetting(QString name, QString human_readable_name, QString description, EnumSettingValue default_value,
                    QSet<EnumSettingValue> possible_values)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_default_value{std::move(default_value)}, m_possible_values{std::move(possible_values)} {
            if (!isValid(m_default_value)) {
                terminate();
            }
        }

      private:
        EnumSettingValue m_default_value;

        QSet<EnumSettingValue> m_possible_values;
    };

    /*
     * A setting of type HistoricServerConnection.
     *
     * @see HistoricServerConnection
     */
    class HistoricServerConnectionSetting : public Setting {
      public:
        // default is nullopt

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] static bool isValid(StorageId /*value*/) {
            return true;
        }

        HistoricServerConnectionSetting(QString name, QString human_readable_name, QString description)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}
    };

    /*
     * A setting of type Certificate.
     *
     * @see Certificate
     */
    class CertificateSetting : public Setting {
      public:
        // default is nullopt

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] static bool isValid(StorageId /*value*/) {
            return true;
        }

        CertificateSetting(QString name, QString human_readable_name, QString description)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)} {}
    };

    /*
     * A setting of type Layout.
     *
     * @see Layout
     */
    class LayoutSetting : public Setting {
      public:
        /**
         *  @return default value.
         */
        [[nodiscard]] LayoutGroup getGroup() const {
            return m_layout_group;
        }

        // default is nullopt

        /**
         *  @return true iff value fulfills this type of setting.
         */
        [[nodiscard]] static bool isValid(StorageId /*value*/) {
            return true;
        }

        LayoutSetting(QString name, QString human_readable_name, QString description, LayoutGroup layout_group)
            : Setting{std::move(name), std::move(human_readable_name), std::move(description)},
              m_layout_group{std::move(layout_group)} {}

      private:
        LayoutGroup m_layout_group;
    };
} // namespace magnesia
