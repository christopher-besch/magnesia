#pragma once
#include "../../Activity.hpp"
#include "../../ActivityMetadata.hpp"
#include "../../Application.hpp"
#include "../../database_types.hpp"
#include "../../qt_version_check.hpp"

#include <QCloseEvent>
#include <QList>
#include <QListWidget>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <qtmetamacros.h>
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::settings_activity {
    /*
     * The SettingsActivity is the only location settings ought to be changed.
     *
     * It is unclosable, launched at application initialization and uncreatable by the user.
     */
    class SettingsActivity : public Activity {
      public:
        explicit SettingsActivity(QWidget* parent = nullptr);

        void closeEvent(QCloseEvent* event) override;

        /*
         * Focus a specific setting domain.
         *
         * This only works when the SettingsActivity tab is open.
         *
         * @domain the domain to focus
         * @return true iff the domain was found and focused.
         */
        bool focusDomain(const Domain& domain);
        /*
         * Focus a specific setting.
         *
         * This only works when the SettingsActivity tab is open.
         *
         * @key the key of the setting to focus
         * @return true iff the setting was found and focused.
         */
        bool focusSetting(const SettingKey& key);
        // TODO: maybe focus specific certificate and keys too

      public slots:
        /*
         * To be called when a Certificate was changed
         *
         * @param cert_id the changed Certificate's Id
         *
         * @see StorageManager::certificateChanged
         */
        void onCertificateChange(StorageId cert_id);
        /*
         * To be called when a Key was changed
         *
         * @param key_id the changed Key's Id
         *
         * @see StorageManager::keyChanged
         */
        void onKeyChange(StorageId key_id);
        /*
         * To be called when a Layout was changed
         *
         * @param layout_id the changed Layout's Id
         *
         * @see StorageManager::layoutChanged
         */
        void onLayoutChange(StorageId layout_id);
        /*
         * To be called when a HistoricConnection was changed
         *
         * @param server_con_id the changed HistoricConnection's Id
         *
         * @see StorageManager::historicConnectionChanged
         */
        void onHistoricConnectionChange(StorageId server_con_id);
        // onKVChanged is not needed because there is no KVSetting and Key-Value pairs are not displayed in the Activity
        // onSettingChanged is not needed as this is the only location settings ought to be changed
        /*
         * To be called when a setting domain was defined.
         *
         * @param domain the domain that was defined.
         *
         * @see SettingsManager::settingDomainDefined
         */
        void onSettingDomainDefined(const Domain& domain);

      private:
        void reRenderSettings();
        void reRenderCertificates();
        void reRenderKeys();

        void focusDomain(int index);
        void focusCertificates();
        void focusKeys();

      private:
        // list of all links in sidebar
        QListWidget* m_sidebar_domain_list{nullptr};
        // scroll area on the right, used to jump to specific widgets
        QScrollArea* m_scroll_area{nullptr};
        // layout where the settings for each domain are
        QVBoxLayout* m_domain_list{nullptr};
        // layout where all certificate are
        QVBoxLayout* m_certificate_list{nullptr};
        // layout where all keys are
        QVBoxLayout* m_key_list{nullptr};

        // only used to focus specific widgets and delete them
        QMap<Domain, QWidget*>     m_domain_widgets;
        QMap<SettingKey, QWidget*> m_setting_widgets;
        QList<QWidget*>            m_certificate_widgets;
        QList<QWidget*>            m_key_widgets;
        QWidget*                   m_certificates_widget{nullptr};
        QWidget*                   m_keys_widget{nullptr};
    };

    inline constexpr ActivityMetadata metadata{
        .name = u"Settings Activity",
        .global_init =
            []() {
                // TODO: uncomment false
                Application::instance().openActivity(new SettingsActivity, "Settings" /*, false*/);
                qDebug() << "Initialized SettingsActivity";
            },
    };
} // namespace magnesia::activities::settings_activity
