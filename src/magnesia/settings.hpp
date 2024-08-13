#pragma once

#include "database_types.hpp"

#include <cstdint>

#include <QLayout>
#include <QSet>
#include <QString>
#include <QWidget>

namespace magnesia {
    namespace activities::settings_activity {
        class SettingsActivity;
    } // namespace activities::settings_activity

    /**
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

        [[nodiscard]] QString getName() const;
        [[nodiscard]] QString getHumanReadableName() const;
        [[nodiscard]] QString getDescription() const;

      protected:
        /**
         * @param name The key to identify this setting inside its domain.
         * @param human_readable_name The name displayed to the user.
         * @param description The description displayed to the user.
         */
        Setting(QString name, QString human_readable_name, QString description);
        Setting(const Setting&)            = default;
        Setting(Setting&&)                 = default;
        Setting& operator=(const Setting&) = default;
        Setting& operator=(Setting&&)      = default;

      private:
        QString m_name;
        QString m_human_readable_name;
        QString m_description;
    };

    /**
     * A setting of type bool.
     */
    class BooleanSetting : public Setting {
      public:
        /**
         *  @return the default value.
         */
        [[nodiscard]] bool getDefault() const;

        /**
         *  @return true iff value fulfills the requirements for boolean settings.
         */
        [[nodiscard]] static bool isValid(bool value);

        BooleanSetting(QString name, QString human_readable_name, QString description, bool default_value);

      private:
        bool m_default_value;
    };

    /**
     * A setting of type string represented as QString.
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

        StringSetting(QString name, QString human_readable_name, QString description, QString default_value);

      private:
        QString m_default_value;
    };

    /**
     * A setting of type int in [min, max].
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

        IntSetting(QString name, QString human_readable_name, QString description, std::int64_t default_value,
                   std::int64_t min, std::int64_t max);

      private:
        std::int64_t m_default_value;
        std::int64_t m_min;
        std::int64_t m_max;
    };

    /**
     * A setting of type double in [min, max].
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
         *  @return true iff value fulfills the requirements for double settings.
         */
        [[nodiscard]] bool isValid(double value) const;

        DoubleSetting(QString name, QString human_readable_name, QString description, double default_value, double min,
                      double max);

      private:
        double m_default_value;
        double m_min;
        double m_max;
    };

    /**
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
        [[nodiscard]] const QSet<EnumSettingValue>& getPossibleValues() const;

        /**
         *  @return true iff value fulfills the requirements for enum settings.
         */
        [[nodiscard]] bool isValid(const EnumSettingValue& value) const;

        EnumSetting(QString name, QString human_readable_name, QString description, EnumSettingValue default_value,
                    QSet<EnumSettingValue> possible_values);

      private:
        EnumSettingValue       m_default_value;
        QSet<EnumSettingValue> m_possible_values;
    };

    /**
     * A setting of type HistoricServerConnection.
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
         *  @return true iff value fulfills the requirements for historic server connection settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        HistoricServerConnectionSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * A setting of type Certificate.
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
         *  @return true iff value fulfills the requirements for certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        CertificateSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * A setting of type Key.
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
         *  @return true iff value fulfills the requirements for certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        KeySetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * A setting of type ApplicationCertificate.
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
         *  @return true iff value fulfills the requirements for application certificate settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        ApplicationCertificateSetting(QString name, QString human_readable_name, QString description);
    };

    /**
     * A setting of type Layout.
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
         *  @return true iff value fulfills the requirements for layout settings.
         */
        [[nodiscard]] static bool isValid(StorageId value);

        LayoutSetting(QString name, QString human_readable_name, QString description, LayoutGroup layout_group);

      private:
        LayoutGroup m_layout_group;
    };
} // namespace magnesia
