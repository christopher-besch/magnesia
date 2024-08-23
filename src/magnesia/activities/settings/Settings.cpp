#include "Settings.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../qt_version_check.hpp"
#include "../../settings.hpp"
#include "../../terminate.hpp"
#include "SettingsUrlHandler.hpp"

#include <cstddef>
#include <functional>
#include <ranges>

#include <QAbstractItemView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QLoggingCategory>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QtAlgorithms>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_settings, "magnesia.settings")
} // namespace

namespace magnesia::activities::settings {
    // TODO: combine with implementation in dataviewer
    namespace {
        QWidget* wrap_in_frame(QLayout* layout) {
            auto* frame = new QFrame;
            frame->setLayout(layout);
            frame->setFrameShape(QFrame::Shape::Box);
            return frame;
        }

        QWidget* wrap_in_setting_widget(QLayout* right_layout, const Setting* setting) {
            // left section
            auto* left_layout = new QVBoxLayout;
            left_layout->addWidget(new QLabel{setting->getHumanReadableName()});
            left_layout->addWidget(new QLabel{QString{"<em>%1</em>"}.arg(setting->getDescription())});

            // main setting widget
            auto* main_layout = new QHBoxLayout;
            main_layout->addLayout(left_layout, 1);
            main_layout->addLayout(right_layout, 1);

            auto* widget = new QWidget;
            widget->setLayout(main_layout);
            return widget;
        }

        QWidget* wrap_in_setting_widget(QLayout* right_layout, const Setting* setting,
                                        const std::function<void()>& on_reset, const SettingKey& reset_key) {
            auto* reset_button = new QPushButton{"Reset"};
            QObject::connect(reset_button, &QPushButton::clicked, reset_button, [on_reset, reset_key] {
                auto success = Application::instance().getSettingsManager().resetSetting(reset_key);
                Q_ASSERT(success);
                std::invoke(on_reset);
            });
            right_layout->addWidget(reset_button);
            return wrap_in_setting_widget(right_layout, setting);
        }
    } // namespace

    Settings::Settings(QWidget* parent)
        : Activity{parent}, m_sidebar_domain_list{new QListWidget}, m_scroll_area{new QScrollArea},
          m_domain_list{new QVBoxLayout}, m_certificate_list{new QVBoxLayout}, m_key_list{new QVBoxLayout} {
        // sidebar (in activity layout)
        m_sidebar_domain_list->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        connect(m_sidebar_domain_list, &QListWidget::itemClicked, this, [this] {
            const int current_selected = m_sidebar_domain_list->currentRow();
            const int count            = m_sidebar_domain_list->count();
            qCDebug(lc_settings) << "changing to" << current_selected;
            // The last and penultimate items are links to certificates and keys, not a setting domain.
            // When a new special section is added, this needs to be adjusted.
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
        scroll_area_layout->addWidget(new QLabel{"Settings"});
        scroll_area_layout->addLayout(m_domain_list);

        // certificates (in main scroll area)
        auto* certificates_layout = new QVBoxLayout;
        certificates_layout->addWidget(new QLabel{"Certificates"});
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
        connect(create_app_cert, &QPushButton::clicked, this,
                []() { Application::instance().getRouter().route({"certificate:create"}); });
        scroll_area_layout->addWidget(create_app_cert);

        // activity layout
        auto* layout = new QHBoxLayout;
        layout->setContentsMargins(4, 4, 4, 4);
        layout->addWidget(m_sidebar_domain_list, 1);
        layout->addWidget(m_scroll_area, 5); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        setLayout(layout);

        Application::instance().getRouter().registerDynamicSchemeHandler(
            "settings",
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete): https://github.com/llvm/llvm-project/issues/62985
            new SettingsUrlHandler{this});

        reCreateSettings();
        reCreateCertificates();
        reCreateKeys();

        auto* storage_manager = &Application::instance().getStorageManager();
        connect(storage_manager, &StorageManager::certificateChanged, this, &Settings::onCertificateChange);
        connect(storage_manager, &StorageManager::keyChanged, this, &Settings::onKeyChange);
        connect(storage_manager, &StorageManager::applicationCertificateChanged, this,
                &Settings::onApplicationCertificateChange);
        connect(storage_manager, &StorageManager::layoutChanged, this, &Settings::onLayoutChange);
        connect(storage_manager, &StorageManager::historicServerConnectionChanged, this,
                &Settings::onHistoricServerConnectionChange);

        auto* settings_manager = &Application::instance().getSettingsManager();
        connect(settings_manager, &SettingsManager::settingDomainDefined, this, &Settings::onSettingDomainDefined);
        connect(settings_manager, &SettingsManager::settingChanged, this, &Settings::onSettingChanged);
    }

    void Settings::onCertificateChange(StorageId /*cert_id*/) {
        setUpdatesEnabled(false);
        reCreateCertificates();
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onKeyChange(StorageId /*key_id*/) {
        setUpdatesEnabled(false);
        reCreateKeys();
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onApplicationCertificateChange(StorageId /*cert_id*/) {
        setUpdatesEnabled(false);
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onLayoutChange(StorageId /*layout_id*/) {
        setUpdatesEnabled(false);
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onHistoricServerConnectionChange(StorageId /*server_con_id*/) {
        setUpdatesEnabled(false);
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onSettingDomainDefined(const Domain& /*domain*/) {
        setUpdatesEnabled(false);
        reCreateSettings();
        setUpdatesEnabled(true);
    }

    void Settings::onSettingChanged(const SettingKey& /*key*/) {
        // TODO: do this better
        // heuristic: when this tab is visible the user probably didn't change anything somewhere else and this event is
        // coming from the Settings itself
        if (!isVisible()) {
            setUpdatesEnabled(false);
            reCreateSettings();
            setUpdatesEnabled(true);
        }
    }

    void Settings::reCreateSettings() {
        // delete old settings UI
        m_sidebar_domain_list->clear();
        // TODO: use smart pointers instead
        qDeleteAll(m_domain_widgets | std::views::transform([](const auto& pair) { return pair.second; }));
        m_domain_widgets.clear();
        // All setting widgets are children of domains, which are deleted above.
        m_setting_widgets.clear();

        // create new settings UI
        for (const Domain& domain : Application::instance().getSettingsManager().getAllDomains()) {
            // sidebar
            m_sidebar_domain_list->addItem(domain);

            // domain in main area
            auto* domain_layout = new QVBoxLayout;

            auto* domain_heading = new QLabel{domain};
            domain_layout->addWidget(domain_heading);

            auto* wrapped_domain_widget = wrap_in_frame(domain_layout);
            m_domain_list->addWidget(wrapped_domain_widget);
            m_domain_widgets[domain] = wrapped_domain_widget;

            // settings inside domain
            for (const auto& setting : Application::instance().getSettingsManager().getSettingDefinitions(domain)) {
                auto* setting_widget = createSettingWidget(setting.get(), domain);
                domain_layout->addWidget(setting_widget);
                m_setting_widgets[{.name = setting->getName(), .domain = domain}] = setting_widget;
            }
        }
        // these need to go last
        m_sidebar_domain_list->addItem("Certificates");
        m_sidebar_domain_list->addItem("Keys");
    }

    void Settings::reCreateCertificates() {
        qDeleteAll(m_certificate_widgets);
        m_certificate_widgets.clear();

        for (const auto& [cert_id, cert] : Application::instance().getStorageManager().getAllCertificates()) {
            auto* layout = new QHBoxLayout;

            auto* label = new QLabel{cert.subjectDisplayName()};
            layout->addWidget(label);

            auto* view_button = new QPushButton{"View"};
            layout->addWidget(view_button);
            connect(view_button, &QPushButton::clicked, view_button, [cert_id]() {
                Application::instance().getRouter().route("certificate:view?storage-id=" + QString::number(cert_id));
            });

            auto* delete_button = new QPushButton{"Delete"};
            layout->addWidget(delete_button);
            connect(delete_button, &QPushButton::clicked, delete_button,
                    [cert_id]() { Application::instance().getStorageManager().deleteCertificate(cert_id); });

            auto* cert_widget = new QWidget;
            cert_widget->setLayout(layout);
            m_certificate_list->addWidget(cert_widget);
            m_certificate_widgets.push_back(cert_widget);
        }
    }

    void Settings::reCreateKeys() {
        qDeleteAll(m_key_widgets);
        m_key_widgets.clear();

        for (const auto& [key_id, _] : Application::instance().getStorageManager().getAllKeys()) {
            auto* layout = new QHBoxLayout;

            // TODO: figure out a better label
            auto* label = new QLabel{QString::number(key_id)};
            layout->addWidget(label);

            auto* delete_button = new QPushButton{"Delete"};
            layout->addWidget(delete_button);
            connect(delete_button, &QPushButton::clicked, delete_button,
                    [key_id]() { Application::instance().getStorageManager().deleteKey(key_id); });

            auto* key_widget = new QWidget;
            key_widget->setLayout(layout);
            m_key_list->addWidget(key_widget);
            m_key_widgets.push_back(key_widget);
        }
    }

    bool Settings::focusDomain(const Domain& domain) {
        if (m_domain_widgets.contains(domain)) {
            qCDebug(lc_settings) << "focus domain:" << domain;
            m_scroll_area->ensureWidgetVisible(m_domain_widgets[domain]);
            return true;
        }
        qCWarning(lc_settings) << "can't focus domain:" << domain;
        return false;
    }

    bool Settings::focusSetting(const SettingKey& key) {
        if (m_setting_widgets.contains(key)) {
            m_scroll_area->ensureWidgetVisible(m_setting_widgets[key]);
            qCDebug(lc_settings) << "focus setting:" << key.domain << key.name;
            return true;
        }
        qCWarning(lc_settings) << "can't focus setting:" << key.domain << key.name;
        return false;
    }

    void Settings::focusDomain(int index) {
        const auto& domains = Application::instance().getSettingsManager().getAllDomains();
        focusDomain(domains[static_cast<std::size_t>(index)]);
    }

    void Settings::focusCertificates() {
        m_scroll_area->ensureWidgetVisible(m_certificates_widget);
    }

    void Settings::focusKeys() {
        m_scroll_area->ensureWidgetVisible(m_keys_widget);
    }

    QWidget* Settings::createSettingWidget(const Setting* setting, const Domain& domain) {
        if (const auto* specific_setting = dynamic_cast<const BooleanSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const StringSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const IntSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const DoubleSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const EnumSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const HistoricServerConnectionSetting*>(setting);
            specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const CertificateSetting*>(setting);
            specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const KeySetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const ApplicationCertificateSetting*>(setting);
            specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        if (const auto* specific_setting = dynamic_cast<const LayoutSetting*>(setting); specific_setting != nullptr) {
            return createSettingWidget(specific_setting, domain);
        }
        qCCritical(lc_settings) << "failed to create widget for unknown Setting type";
        terminate();
    }

    QWidget* Settings::createSettingWidget(const BooleanSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getBoolSetting(key);
        Q_ASSERT(cur_setting_value.has_value());

        auto* button = new QPushButton{cur_setting_value.value() ? "True" : "False"};
        button->setCheckable(true);
        button->setChecked(cur_setting_value.value());
        right_layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, button, [button, key](bool checked) {
            Application::instance().getSettingsManager().setBooleanSetting(key, checked);
            button->setText(checked ? "True" : "False");
        });

        // Using this as a pointer in the on_reset closure is fine as settings are never removed (or moved) without the
        // Widget being reconstructed.
        return wrap_in_setting_widget(
            right_layout, setting,
            [setting, button]() {
                button->setChecked(setting->getDefault());
                button->setText(setting->getDefault() ? "True" : "False");
            },
            key);
    }

    QWidget* Settings::createSettingWidget(const StringSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getStringSetting(key);
        Q_ASSERT(cur_setting_value.has_value());

        auto* line_edit = new QLineEdit{cur_setting_value.value()};
        right_layout->addWidget(line_edit);
        QObject::connect(line_edit, &QLineEdit::editingFinished, line_edit, [line_edit, key]() {
            Application::instance().getSettingsManager().setStringSetting(key, line_edit->text());
        });

        return wrap_in_setting_widget(
            right_layout, setting, [line_edit, setting]() { line_edit->setText(setting->getDefault()); }, key);
    }

    QWidget* Settings::createSettingWidget(const IntSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getIntSetting(key);
        Q_ASSERT(cur_setting_value.has_value());

        auto* spin_box = new QSpinBox;
        spin_box->setMinimum(static_cast<int>(setting->getMin()));
        spin_box->setMaximum(static_cast<int>(setting->getMax()));
        spin_box->setValue(static_cast<int>(cur_setting_value.value()));
        right_layout->addWidget(spin_box);
        right_layout->addWidget(new QLabel{QString{"min: %1 max: %2"}.arg(setting->getMin()).arg(setting->getMax())});
        QObject::connect(spin_box, &QSpinBox::valueChanged, spin_box,
                         [key](int value) { Application::instance().getSettingsManager().setIntSetting(key, value); });

        return wrap_in_setting_widget(
            right_layout, setting,
            [spin_box, setting]() { spin_box->setValue(static_cast<int>(setting->getDefault())); }, key);
    }

    QWidget* Settings::createSettingWidget(const DoubleSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getDoubleSetting(key);
        Q_ASSERT(cur_setting_value.has_value());

        auto* spin_box = new QDoubleSpinBox;
        spin_box->setMinimum(setting->getMin());
        spin_box->setMaximum(setting->getMax());
        spin_box->setValue(cur_setting_value.value());
        right_layout->addWidget(spin_box);
        right_layout->addWidget(new QLabel{QString{"min: %1 max: %2"}.arg(setting->getMin()).arg(setting->getMax())});
        QObject::connect(spin_box, &QDoubleSpinBox::valueChanged, right_layout, [key](double value) {
            Application::instance().getSettingsManager().setDoubleSetting(key, value);
        });

        return wrap_in_setting_widget(
            right_layout, setting, [spin_box, setting]() { spin_box->setValue(setting->getDefault()); }, key);
    }

    QWidget* Settings::createSettingWidget(const EnumSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getEnumSetting(key);
        Q_ASSERT(cur_setting_value.has_value());

        auto* combo_box = new QComboBox;
        for (const auto& possible_value : setting->getPossibleValues()) {
            combo_box->addItem(possible_value);
        }
        combo_box->setCurrentText(cur_setting_value.value());
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentTextChanged, combo_box, [key](const QString& text) {
            Application::instance().getSettingsManager().setEnumSetting(key, text);
        });

        return wrap_in_setting_widget(
            right_layout, setting, [combo_box, setting]() { combo_box->setCurrentText(setting->getDefault()); }, key);
    }

    QWidget* Settings::createSettingWidget(const HistoricServerConnectionSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value =
            Application::instance().getSettingsManager().getHistoricServerConnectionSettingId(key);
        const auto server_cons = Application::instance().getStorageManager().getAllHistoricServerConnections();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& [server_con_id, server_con] : server_cons) {
            // TODO: better text, maybe include timestamp
            combo_box->addItem(server_con.endpoint_url.toString(), server_con_id);
            if (cur_setting_value.has_value() && cur_setting_value.value() == server_con_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, combo_box, [combo_box, key](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setHistoricServerConnectionSetting(
                key, combo_box->itemData(index).value<StorageId>());
        });
        return wrap_in_setting_widget(right_layout, setting);
    }

    QWidget* Settings::createSettingWidget(const CertificateSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getCertificateSettingId(key);
        const auto       certs             = Application::instance().getStorageManager().getAllCertificates();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& [cert_id, cert] : certs) {
            combo_box->addItem(cert.subjectDisplayName(), cert_id);
            if (cur_setting_value.has_value() && cur_setting_value.value() == cert_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, combo_box, [combo_box, key](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setCertificateSetting(
                key, combo_box->itemData(index).value<StorageId>());
        });
        return wrap_in_setting_widget(right_layout, setting);
    }

    QWidget* Settings::createSettingWidget(const KeySetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getKeySettingId(key);
        const auto       keys              = Application::instance().getStorageManager().getAllKeys();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& [key_id, _] : keys) {
            const auto& ssl_key = Application::instance().getStorageManager().getKey(key_id);
            Q_ASSERT(ssl_key.has_value());
            // TODO: find better name
            combo_box->addItem(QString::number(ssl_key.value().length()), key_id);
            if (cur_setting_value.has_value() && cur_setting_value.value() == key_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, combo_box, [combo_box, key](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setKeySetting(key,
                                                                       combo_box->itemData(index).value<StorageId>());
        });
        return wrap_in_setting_widget(right_layout, setting);
    }

    QWidget* Settings::createSettingWidget(const ApplicationCertificateSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value =
            Application::instance().getSettingsManager().getApplicationCertificateSettingId(key);
        const auto certs = Application::instance().getStorageManager().getAllApplicationCertificates();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& [cert_id, cert] : certs) {
            combo_box->addItem(cert.getCertificate().subjectDisplayName(), cert_id);
            if (cur_setting_value.has_value() && cur_setting_value.value() == cert_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, combo_box, [combo_box, key](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setApplicationCertificateSetting(
                key, combo_box->itemData(index).value<StorageId>());
        });
        return wrap_in_setting_widget(right_layout, setting);
    }

    QWidget* Settings::createSettingWidget(const LayoutSetting* setting, const Domain& domain) {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{.name = setting->getName(), .domain = domain};
        const auto       cur_setting_value = Application::instance().getSettingsManager().getLayoutSettingId(key);
        const auto layouts = Application::instance().getStorageManager().getAllLayouts(setting->getGroup(), domain);

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& [layout_id, layout] : layouts) {
            combo_box->addItem(layout.name, layout_id);
            if (cur_setting_value.has_value() && cur_setting_value.value() == layout_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, combo_box, [combo_box, key](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setLayoutSetting(
                key, combo_box->itemData(index).value<StorageId>());
        });
        return wrap_in_setting_widget(right_layout, setting);
    }
} // namespace magnesia::activities::settings
