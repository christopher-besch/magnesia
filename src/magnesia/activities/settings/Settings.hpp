#pragma once

#include "../../Activity.hpp"
#include "../../ActivityMetadata.hpp"
#include "../../Application.hpp"
#include "../../database_types.hpp"
#include "../../qt_version_check.hpp"
#include "../../settings.hpp"

#include <map>

#include <QList>
#include <QListWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::settings {
    /**
     * The Settings activity allows the user to view and alter settings, certificates and keys.
     * It is unclosable, launched at application initialization and not manually creatable by the user.
     */
    class Settings : public Activity {
      public:
        explicit Settings(QWidget* parent = nullptr);

        /**
         * Focus a specific setting domain.
         *
         * This only works when the SettingsActivity tab is open.
         *
         * @param domain the domain to focus
         * @return true iff the domain was found and focused.
         */
        bool focusDomain(const Domain& domain);
        /**
         * Focus a specific setting.
         *
         * This only works when the SettingsActivity tab is open.
         *
         * @param key the key of the setting to focus
         * @return true iff the setting was found and focused.
         */
        bool focusSetting(const SettingKey& key);

        // TODO: maybe focus specific certificate and keys too

      public slots:
        /**
         * To be called when a Certificate was changed
         *
         * @param cert_id the changed Certificate's Id
         *
         * @see StorageManager::certificateChanged
         */
        void onCertificateChange(StorageId cert_id);
        /**
         * To be called when a Key was changed
         *
         * @param key_id the changed Key's Id
         *
         * @see StorageManager::keyChanged
         */
        void onKeyChange(StorageId key_id);
        /**
         * To be called when an ApplicationCertificate was changed
         *
         * @param cert_id the changed ApplicationCertificate's Id
         *
         * @see StorageManager::applicationCertificateChanged
         */
        void onApplicationCertificateChange(StorageId cert_id);
        /**
         * To be called when a Layout was changed
         *
         * @param layout_id the changed Layout's Id
         *
         * @see StorageManager::layoutChanged
         */
        void onLayoutChange(StorageId layout_id);
        /**
         * To be called when a HistoricServerConnection was changed
         *
         * @param server_con_id the changed HistoricServerConnection's Id
         *
         * @see StorageManager::historicServerConnectionChanged
         */
        void onHistoricServerConnectionChange(StorageId server_con_id);
        /**
         * To be called when a setting domain was defined.
         *
         * @param domain the domain that was defined.
         *
         * @see SettingsManager::settingDomainDefined
         */
        void onSettingDomainDefined(const Domain& domain);
        /**
         * To be called when a Setting was changed
         *
         * @param key the key of the Setting that changed
         *
         * @see StorageManager::settingChanged
         */
        void onSettingChanged(const SettingKey& key);

      private:
        void reCreateSettings();
        void reCreateCertificates();
        void reCreateKeys();

        void focusDomain(int index);
        void focusCertificates();
        void focusKeys();

        static QWidget* createSettingWidget(const Setting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const BooleanSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const StringSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const IntSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const DoubleSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const EnumSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const HistoricServerConnectionSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const CertificateSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const KeySetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const ApplicationCertificateSetting* setting, const Domain& domain);
        static QWidget* createSettingWidget(const LayoutSetting* setting, const Domain& domain);

      private:
        // list of all links in sidebar
        QListWidget* m_sidebar_domain_list{nullptr};
        // scroll area containing the settings widgets
        QScrollArea* m_scroll_area{nullptr};
        // layout where the settings for each domain are
        QVBoxLayout* m_domain_list{nullptr};
        // layout where all certificate are
        QVBoxLayout* m_certificate_list{nullptr};
        // layout where all keys are
        QVBoxLayout* m_key_list{nullptr};

        // only used to focus specific widgets and delete them
        std::map<Domain, QWidget*>     m_domain_widgets;
        std::map<SettingKey, QWidget*> m_setting_widgets;
        QList<QWidget*>                m_certificate_widgets;
        QList<QWidget*>                m_key_widgets;
        QWidget*                       m_certificates_widget{nullptr};
        QWidget*                       m_keys_widget{nullptr};
    };

    inline constexpr ActivityMetadata metadata{
        .name        = u"Settings",
        .global_init = [] { Application::instance().openActivity(new Settings, "Settings", false); },
    };
} // namespace magnesia::activities::settings
