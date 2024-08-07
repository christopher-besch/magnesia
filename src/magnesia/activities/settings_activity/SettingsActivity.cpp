#include "SettingsActivity.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../qt_version_check.hpp"
#include "../../url_handlers/SettingsUrlHandler.hpp"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QtAlgorithms>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::settings_activity {
    // TODO: combine with implementation in dataviewer
    QWidget* wrap_in_frame(QLayout* layout) {
        auto* frame = new QFrame;
        frame->setLayout(layout);
        frame->setFrameShape(QFrame::Shape::Box);
        return frame;
    }

    QWidget* wrap_in_frame(QWidget* widget) {
        auto* layout = new QVBoxLayout;
        layout->addWidget(widget);
        return wrap_in_frame(layout);
    }

    SettingsActivity::SettingsActivity(QWidget* parent)
        : Activity{parent}, m_sidebar_domain_list{new QListWidget}, m_scroll_area{new QScrollArea},
          m_domain_list{new QVBoxLayout}, m_certificate_list{new QVBoxLayout}, m_key_list{new QVBoxLayout} {

        // sidebar (in activity layout)
        m_sidebar_domain_list->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        connect(m_sidebar_domain_list, &QListWidget::itemClicked, [&](QListWidgetItem*) {
            const int current_selected = m_sidebar_domain_list->currentRow();
            const int count            = m_sidebar_domain_list->count();
            qDebug() << "changing to" << current_selected;
            switch (count - current_selected) {
                case 2:
                    focusCertificates();
                    return;
                case 1:
                    focusKeys();
                    return;
                default:
                    focusDomain(current_selected);
                    return;
            }
        });

        // main scroll area (in activity layout)
        auto* scroll_area_layout = new QVBoxLayout;
        m_scroll_area->setWidgetResizable(true);
        auto* scroll_area_widget = new QWidget;
        scroll_area_widget->setLayout(scroll_area_layout);
        m_scroll_area->setWidget(scroll_area_widget);

        // settings (in main scroll area)
        auto* settings_heading = new QLabel{"Settings"};
        scroll_area_layout->addWidget(settings_heading);
        scroll_area_layout->addLayout(m_domain_list);

        // certificates (in main scroll area)
        auto* certificates_heading = new QLabel{"Certificates"};
        auto* certificates_layout  = new QVBoxLayout;
        certificates_layout->addWidget(certificates_heading);
        certificates_layout->addLayout(m_certificate_list);
        m_certificates_widget = wrap_in_frame(certificates_layout);
        scroll_area_layout->addWidget(m_certificates_widget);

        // keys (in main scroll area)
        auto* keys_heading = new QLabel{"Keys"};
        auto* keys_layout  = new QVBoxLayout;
        keys_layout->addWidget(keys_heading);
        keys_layout->addLayout(m_key_list);
        m_keys_widget = wrap_in_frame(keys_layout);
        scroll_area_layout->addWidget(m_keys_widget);

        // create ApplicationCertificate (in main scroll area)
        auto* create_app_cert = new QPushButton{"Create New Application Certificate"};
        connect(create_app_cert, &QPushButton::clicked, []() {
            // TODO: use correct url when the certificate activity is implemented
            Application::instance().getRouter().route(QUrl{""});
        });
        scroll_area_layout->addWidget(create_app_cert);

        // activity layout
        auto* layout = new QHBoxLayout{this};
        layout->addWidget(m_sidebar_domain_list, 1);
        layout->addWidget(m_scroll_area, 5); // NOLINT: cppcoreguidelines-avoid-magic-numbers
        setLayout(layout);

        Application::instance().getRouter().registerDynamicSchemeHandler(
            "settings:settings",
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete): https://github.com/llvm/llvm-project/issues/62985
            new url_handlers::SettingsUrlHandler(this));

        reRenderSettings();
        reRenderCertificates();
        reRenderKeys();

        connect(&Application::instance().getStorageManager(), &StorageManager::certificateChanged, this,
                &SettingsActivity::onCertificateChange);
        connect(&Application::instance().getStorageManager(), &StorageManager::keyChanged, this,
                &SettingsActivity::onKeyChange);
        connect(&Application::instance().getStorageManager(), &StorageManager::layoutChanged, this,
                &SettingsActivity::onLayoutChange);
        connect(&Application::instance().getStorageManager(), &StorageManager::historicConnectionChanged, this,
                &SettingsActivity::onHistoricConnectionChange);
        connect(&Application::instance().getSettingsManager(), &SettingsManager::settingDomainDefined, this,
                &SettingsActivity::onSettingDomainDefined);
    }

    void SettingsActivity::closeEvent(QCloseEvent* event) {
        Q_ASSERT(event != nullptr);
        // This Activity cannot be closed.
        qDebug() << "SettingsActivity: ignoring CloseEvent";
        event->ignore();
    }

    void SettingsActivity::onCertificateChange(StorageId /*cert_id*/) {
        reRenderCertificates();
        reRenderSettings();
    }

    void SettingsActivity::onKeyChange(StorageId /*key_id*/) {
        reRenderKeys();
        reRenderSettings();
    }

    void SettingsActivity::onLayoutChange(StorageId /*layout_id*/) {
        reRenderSettings();
    }

    void SettingsActivity::onHistoricConnectionChange(StorageId /*server_con_id*/) {
        reRenderSettings();
    }

    void SettingsActivity::onSettingDomainDefined(const Domain& /*domain*/) {
        reRenderSettings();
    }

    void SettingsActivity::reRenderSettings() {
        // delete old settings UI
        m_sidebar_domain_list->clear();
        qDeleteAll(m_domain_widgets);
        m_domain_widgets.clear();
        m_setting_widgets.clear();

        // create new settings UI
        for (const Domain& domain : Application::instance().getSettingsManager().getAllDomains()) {
            // sidebar
            m_sidebar_domain_list->addItem(domain);

            // domain in main area
            auto* domain_layout = new QVBoxLayout;
            domain_layout->setContentsMargins(0, 0, 0, 0);

            auto* domain_heading = new QLabel{domain};
            domain_layout->addWidget(domain_heading);

            auto* domain_widget = new QWidget;
            domain_widget->setLayout(domain_layout);
            auto* wrapped_domain_widget = wrap_in_frame(domain_widget);
            m_domain_list->addWidget(wrapped_domain_widget);
            m_domain_widgets[domain] = wrapped_domain_widget;

            // settings inside domain
            for (const auto& setting : Application::instance().getSettingsManager().getSettingDefinitions(domain)) {
                auto* setting_widget = setting->constructWidget(domain);
                domain_layout->addWidget(setting_widget);
                m_setting_widgets[{domain, setting->getName()}] = setting_widget;
            }
        }
        // these need to go last
        m_sidebar_domain_list->addItem("Certificates");
        m_sidebar_domain_list->addItem("Keys");
    }

    void SettingsActivity::reRenderCertificates() {
        qDeleteAll(m_certificate_widgets);
        m_certificate_widgets.clear();

        for (const StorageId cert_id : Application::instance().getStorageManager().getAllCertificateIds()) {
            auto cert = Application::instance().getStorageManager().getCertificate(cert_id);
            Q_ASSERT(cert.has_value());
            auto* layout = new QHBoxLayout;

            auto* label = new QLabel{cert.value().subjectDisplayName()};
            layout->addWidget(label);

            auto* view_button = new QPushButton{"View"};
            layout->addWidget(view_button);
            connect(view_button, &QPushButton::clicked, [/*cert_id*/]() {
                // TODO: use correct url when the certificate activity is implemented
                Application::instance().getRouter().route(QUrl{""});
            });

            auto* delete_button = new QPushButton{"Delete"};
            layout->addWidget(delete_button);
            connect(delete_button, &QPushButton::clicked,
                    [cert_id]() { Application::instance().getStorageManager().deleteCertificate(cert_id); });

            auto* cert_widget = new QWidget;
            cert_widget->setLayout(layout);
            m_certificate_list->addWidget(cert_widget);
            m_certificate_widgets.append(cert_widget);
        }
    }

    void SettingsActivity::reRenderKeys() {
        qDeleteAll(m_key_widgets);
        m_key_widgets.clear();

        for (const StorageId key_id : Application::instance().getStorageManager().getAllKeyIds()) {
            auto key = Application::instance().getStorageManager().getKey(key_id);
            Q_ASSERT(key.has_value());
            auto* layout = new QHBoxLayout;

            // TODO: figure out a better label
            auto* label = new QLabel{QString::number(key.value().length())};
            layout->addWidget(label);

            auto* delete_button = new QPushButton{"Delete"};
            layout->addWidget(delete_button);
            connect(delete_button, &QPushButton::clicked,
                    [key_id]() { Application::instance().getStorageManager().deleteKey(key_id); });

            auto* key_widget = new QWidget;
            key_widget->setLayout(layout);
            m_key_list->addWidget(key_widget);
            m_key_widgets.append(key_widget);
        }
    }

    bool SettingsActivity::focusDomain(const Domain& domain) {
        if (m_domain_widgets.contains(domain)) {
            qDebug() << "focus domain:" << domain;
            m_scroll_area->ensureWidgetVisible(m_domain_widgets[domain]);
            return true;
        }
        qDebug() << "can't focus domain:" << domain;
        return false;
    }

    bool SettingsActivity::focusSetting(const SettingKey& key) {
        if (m_setting_widgets.contains(key)) {
            m_scroll_area->ensureWidgetVisible(m_setting_widgets[key]);
            qDebug() << "focus setting:" << key.domain << key.name;
            return true;
        }
        qDebug() << "can't focus setting:" << key.domain << key.name;
        return false;
    }

    void SettingsActivity::focusDomain(int index) {
        const auto& domains = Application::instance().getSettingsManager().getAllDomains();
        focusDomain(domains[index]);
    }

    void SettingsActivity::focusCertificates() {
        m_scroll_area->ensureWidgetVisible(m_certificates_widget);
    }

    void SettingsActivity::focusKeys() {
        m_scroll_area->ensureWidgetVisible(m_keys_widget);
    }
} // namespace magnesia::activities::settings_activity
