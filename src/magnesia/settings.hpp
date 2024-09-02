#pragma once

#include "database_types.hpp"

#include <cstdint>
#include <set>

#include <QLayout>
#include <QString>
#include <QWidget>

namespace magnesia {
    namespace activities::settings_activity {
        class SettingsActivity;
    } // namespace activities::settings_activity

    /**
     * @class Setting
     * @brief Base class for all Settings definitions.
     *
     * Base class for all Settings definitions.
     *
     * The SettingsManager needs to know what settings to manage.
     * These are defined using subclasses of this base class.
     *
     * These classes don't contain the currently set value for any setting.
     * They only contain static data that isn't changeable by the user.
     */
    class Setting {
      public:
        virtual ~Setting() = default;

        /**
         * Retrieves the name of the setting.
         *
         * @return QString containing the name of the setting.
         */
        [[nodiscard]] QString getName() const;

        /**
         * Retrieves the human readable name of the setting,
         *
         * @return QString containing the human readable name of the setting.
         */
        [[nodiscard]] QString getHumanReadableName() const;

        /**
         * Retrieves the description of the setting.
         *
         * @return QString containing the description of the setting.
         */
        [[nodiscard]] QString getDescription() const;

      protected:
        /**
         * @param name the key to identify this setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         */
        Setting(QString name, QString human_readable_name, QString description);

        /**
         */
        Setting(const Setting&) = default;

        /**
         *  Constructing a Setting from an already existing one by using a ltype reference.
         */
        Setting(Setting&&) = default;

        /**
         *  Constructing a Setting from an already existing one by using a ltype reference.
         */
        Setting& operator=(const Setting&) = default;

        /**
         *  Constructing a Setting from an already existing one by using a ltype reference.
         */
        Setting& operator=(Setting&&) = default;

      private:
        QString m_name;
        QString m_human_readable_name;
        QString m_description;
    };

    /**
     * @class BooleanSetting
     * @brief A setting of type bool.
     */
    class BooleanSetting : public Setting {
      public:
        /**
         *  @return The default value.
         */
        [[nodiscard]] bool getDefault() const;

        /**
         *  @return true iff the value fulfills the requirements for boolean settings.
         */
        [[nodiscard]] static bool isValid(bool value);

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param default_value The default value.
         */
        BooleanSetting(QString name, QString human_readable_name, QString description, bool default_value);

      private:
        bool m_default_value;
    };

    /**
     * @class StringSetting
     * @brief A setting of type string represented as a QString.
     */
    class StringSetting : public Setting {
      public:
        /**
         *  @return the default value.
         */
        [[nodiscard]] QString getDefault() const;

        /**
         *  @return true iff value fulfills the requirements for string settings.
         */
        [[nodiscard]] static bool isValid(const QString& value);

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param default_value The default value.
         */
        StringSetting(QString name, QString human_readable_name, QString description, QString default_value);

      private:
        QString m_default_value;
    };

    /**
     * @class IntSetting
     * @brief A setting of type int in [min, max].
     */
    class IntSetting : public Setting {
      public:
        /**
         *  @return the value.
         */
        [[nodiscard]] std::int64_t getDefault() const;

        /**
         *  @return the min value.
         */
        [[nodiscard]] std::int64_t getMin() const;

        /**
         *  @return the max value.
         */
        [[nodiscard]] std::int64_t getMax() const;

        /**
         *  @return true iff value fulfills the requirements for int settings.
         */
        [[nodiscard]] bool isValid(std::int64_t value) const;

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param default_value The default value.
         * @param min Minimal value of the double.
         * @param max Maximal value of the double.
         */
        IntSetting(QString name, QString human_readable_name, QString description, std::int64_t default_value,
                   std::int64_t min, std::int64_t max);

      private:
        std::int64_t m_default_value;
        std::int64_t m_min;
        std::int64_t m_max;
    };

    /**
     * @class DoubleSetting
     * @brief A setting of type double in [min, max].
     */
    class DoubleSetting : public Setting {
      public:
        /**
         *  @return the value.
         */
        [[nodiscard]] double getDefault() const;
        /**
         *  @return the min value.
         */
        [[nodiscard]] double getMin() const;

        /**
         *  @return the max value.
         */
        [[nodiscard]] double getMax() const;

        /**
         *  @return true iff the value fulfills the requirements for double settings.
         */
        [[nodiscard]] bool isValid(double value) const;

        /**
         * @param name The key to identify this setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param default_value The default value.
         * @param min Minimal value of the double.
         * @param max Maximal value of the double.
         */
        DoubleSetting(QString name, QString human_readable_name, QString description, double default_value, double min,
                      double max);

      private:
        double m_default_value;
        double m_min;
        double m_max;
    };

    /**
     * @class EnumSetting
     * @brief A setting of type enum.
     *
     * A setting of type enum.
     * The setting's value must be one of the possible_values.
     */
    class EnumSetting : public Setting {
      public:
        /**
         *  @return the value.
         */
        [[nodiscard]] EnumSettingValue getDefault() const;

        /**
         *  @return the possible value.
         */
        [[nodiscard]] const std::set<EnumSettingValue>& getPossibleValues() const;

        /**
         *  @return true iff the value fulfills the requirements for enum settings.
         */
        [[nodiscard]] bool isValid(const EnumSettingValue& value) const;

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param default_value The default_value
         * @param possible_values List of possible_values of the enum.
         */
        EnumSetting(QString name, QString human_readable_name, QString description, EnumSettingValue default_value,
                    std::set<EnumSettingValue> possible_values);

      private:
        EnumSettingValue           m_default_value;
        std::set<EnumSettingValue> m_possible_values;
    };

    /**
     * @class HistoricServerConnectionSetting
     * @brief A setting of type HistoricServerConnection.
     *
     * @see HistoricServerConnection
     */
    class HistoricServerConnectionSetting : public Setting {
      public:
        // The default is nullopt.
        // Therefore, no getDefault function is needed.

        /**
         * Checking that a HistoricServerConnection with that id exists is done by the StorageManager.
         *
         *  @return true iff the value fulfills the requirements for historic server connection settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         */
        HistoricServerConnectionSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * @class CertificateSetting
     * @brief A setting of type Certificate.
     *
     * @see QSslCertificate
     */
    class CertificateSetting : public Setting {
      public:
        // The default is nullopt.
        // Therefore, no getDefault function is needed.

        /**
         * Checking that a Certificate with that id exists is done by the StorageManager.
         *
         *  @return true iff the value fulfills the requirements for certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        /**
         * @param name The key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         */
        CertificateSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * @class KeySetting
     * @brief A setting of type Key.
     *
     * @see QSslKey
     */
    class KeySetting : public Setting {
      public:
        // The default is nullopt.
        // Therefore, no getDefault function is needed.

        /**
         * Checking that a Key with that id exists is done by the StorageManager.
         *
         *  @return true iff the value fulfills the requirements for certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        /**
         * @param name the key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         */
        KeySetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * @class ApplicationCertificateSetting
     * @brief A setting of type ApplicationCertificate.
     *
     * @see ApplicationCertificate
     */
    class ApplicationCertificateSetting : public Setting {
      public:
        // The default is nullopt.
        // Therefore, no getDefault function is needed.

        /**
         * Checking that an ApplicationCertificate with that id exists is done by the StorageManager.
         *
         *  @return true iff the value fulfills the requirements for application certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        ApplicationCertificateSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * @class LayoutSetting
     * @brief A setting of type Layout.
     *
     * @see Layout
     */
    class LayoutSetting : public Setting {
      public:
        /**
         *  @return the value.
         */
        [[nodiscard]] LayoutGroup getGroup() const;

        // The default is nullopt.
        // Therefore, no getDefault function is needed.

        /**
         * Checking that a Layout with that id, group and domain exists is done by the StorageManager.
         *
         *  @return true iff the value fulfills the requirements for layout settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        /**
         * @param name the key to identify the setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         * @param layout_group the group the layout is in.
         */
        LayoutSetting(QString name, QString human_readable_name, QString description, LayoutGroup layout_group);

      private:
        LayoutGroup m_layout_group;
    };
} // namespace magnesia
