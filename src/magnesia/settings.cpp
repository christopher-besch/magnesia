#include "settings.hpp"

#include "Application.hpp"
#include "database_types.hpp"
#include "qt_version_check.hpp"
#include "terminate.hpp"

#include <cstdint>
#include <functional>
#include <utility>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtLogging>
#else
#include <QtGlobal>
#endif

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

    QWidget* Setting::wrapInSettingsWidget(QLayout* right_layout, const std::function<void()>& on_reset,
                                           const SettingKey& reset_key) const {
        auto* reset_button = new QPushButton{"Reset"};
        QObject::connect(reset_button, &QPushButton::clicked, [on_reset, reset_key](bool /*checked*/) {
            Application::instance().getSettingsManager().resetSetting(reset_key);
            std::invoke(on_reset);
        });
        right_layout->addWidget(reset_button);
        return wrapInSettingsWidget(right_layout);
    }

    QWidget* Setting::wrapInSettingsWidget(QLayout* right_layout) const {
        // left section
        auto* left_layout = new QVBoxLayout;
        left_layout->addWidget(new QLabel{m_human_readable_name});
        left_layout->addWidget(new QLabel{m_description});

        // main setting widget
        auto* setting_widget      = new QWidget;
        auto* setting_main_layout = new QHBoxLayout;
        setting_widget->setLayout(setting_main_layout);
        setting_main_layout->addLayout(left_layout);
        setting_main_layout->addLayout(right_layout);
        return setting_widget;
    }

    QWidget* BooleanSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getBoolSetting(key);
        Q_ASSERT(value.has_value());

        auto* button = new QPushButton{value.value() ? "True" : "False"};
        button->setCheckable(true);
        button->setChecked(value.value());
        right_layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, [button, key](bool checked) {
            Application::instance().getSettingsManager().setBooleanSetting(key, checked);
            button->setText(checked ? "True" : "False");
        });

        // using this as a pointer is fine as settings are never moved
        return wrapInSettingsWidget(
            right_layout,
            [button, this]() {
                button->setChecked(m_default_value);
                button->setText(m_default_value ? "True" : "False");
            },
            key);
    }

    QWidget* StringSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getStringSetting(key);
        Q_ASSERT(value.has_value());

        auto* line_edit = new QLineEdit{value.value()};
        right_layout->addWidget(line_edit);
        QObject::connect(line_edit, &QLineEdit::editingFinished, [line_edit, key]() {
            Application::instance().getSettingsManager().setStringSetting(key, line_edit->text());
        });

        return wrapInSettingsWidget(right_layout, [line_edit, this]() { line_edit->setText(m_default_value); }, key);
    }

    QWidget* IntSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getIntSetting(key);
        Q_ASSERT(value.has_value());

        auto* spin_box = new QSpinBox;
        spin_box->setMinimum(static_cast<int>(m_min));
        spin_box->setMaximum(static_cast<int>(m_max));
        spin_box->setValue(static_cast<int>(value.value()));
        right_layout->addWidget(spin_box);
        right_layout->addWidget(new QLabel{QString{"min: %1 max: %2"}.arg(m_min).arg(m_max)});
        QObject::connect(spin_box, &QSpinBox::valueChanged,
                         [key](int value) { Application::instance().getSettingsManager().setIntSetting(key, value); });

        return wrapInSettingsWidget(
            right_layout, [spin_box, this]() { spin_box->setValue(static_cast<int>(m_default_value)); }, key);
    }

    QWidget* DoubleSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getDoubleSetting(key);
        Q_ASSERT(value.has_value());

        auto* spin_box = new QDoubleSpinBox;
        spin_box->setMinimum(m_min);
        spin_box->setMaximum(m_max);
        spin_box->setValue(value.value());
        right_layout->addWidget(spin_box);
        right_layout->addWidget(new QLabel{QString{"min: %1 max: %2"}.arg(m_min).arg(m_max)});
        QObject::connect(spin_box, &QDoubleSpinBox::valueChanged, [key](double value) {
            Application::instance().getSettingsManager().setDoubleSetting(key, value);
        });

        return wrapInSettingsWidget(
            right_layout, [spin_box, this]() { spin_box->setValue(static_cast<double>(m_default_value)); }, key);
    }

    QWidget* EnumSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getEnumSetting(key);
        Q_ASSERT(value.has_value());

        auto* combo_box = new QComboBox;
        for (const auto& possible_value : m_possible_values) {
            combo_box->addItem(possible_value);
        }
        combo_box->setCurrentText(value.value());
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentTextChanged, [key](const QString& text) {
            Application::instance().getSettingsManager().setEnumSetting(key, text);
        });

        return wrapInSettingsWidget(
            right_layout, [combo_box, this]() { combo_box->setCurrentText(m_default_value); }, key);
    }

    QWidget* HistoricServerConnectionSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getHistoricServerConnectionSettingId(key);
        const auto server_con_ids = Application::instance().getStorageManager().getAllHistoricServerConnectionIds();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& server_con_id : server_con_ids) {
            const auto& server_con =
                Application::instance().getStorageManager().getHistoricServerConnection(server_con_id);
            Q_ASSERT(server_con.has_value());
            // TODO: better text, maybe include timestamp
            combo_box->addItem(server_con.value().endpoint_url.toString());
            if (value.has_value() && value.value() == server_con_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, [key, server_con_ids](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setHistoricServerConnectionSetting(key,
                                                                                            server_con_ids[index - 1]);
        });
        return wrapInSettingsWidget(right_layout);
    }

    QWidget* CertificateSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value    = Application::instance().getSettingsManager().getCertificateSettingId(key);
        const auto       cert_ids = Application::instance().getStorageManager().getAllCertificateIds();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& cert_id : cert_ids) {
            const auto& cert = Application::instance().getStorageManager().getCertificate(cert_id);
            Q_ASSERT(cert.has_value());
            combo_box->addItem(cert.value().subjectDisplayName());
            if (value.has_value() && value.value() == cert_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, [key, cert_ids](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setCertificateSetting(key, cert_ids[index - 1]);
        });
        return wrapInSettingsWidget(right_layout);
    }

    QWidget* KeySetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value   = Application::instance().getSettingsManager().getKeySettingId(key);
        const auto       key_ids = Application::instance().getStorageManager().getAllKeyIds();

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& key_id : key_ids) {
            const auto& ssl_key = Application::instance().getStorageManager().getKey(key_id);
            Q_ASSERT(ssl_key.has_value());
            // TODO: find better name
            combo_box->addItem(QString::number(ssl_key.value().length()));
            if (value.has_value() && value.value() == key_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, [key, key_ids](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setKeySetting(key, key_ids[index - 1]);
        });
        return wrapInSettingsWidget(right_layout);
    }

    QWidget* LayoutSetting::constructWidget(const Domain& domain) const {
        auto* right_layout = new QVBoxLayout;

        const SettingKey key{getName(), domain};
        const auto       value = Application::instance().getSettingsManager().getLayoutSettingId(key);
        const auto layout_ids  = Application::instance().getStorageManager().getAllLayoutIds(m_layout_group, domain);

        auto* combo_box = new QComboBox;
        // used when nullopt
        combo_box->addItem("None");
        combo_box->setCurrentIndex(0);
        for (const auto& layout_id : layout_ids) {
            const auto& layout =
                Application::instance().getStorageManager().getLayout(layout_id, m_layout_group, domain);
            Q_ASSERT(layout.has_value());
            combo_box->addItem(layout.value().name);
            if (value.has_value() && value.value() == layout_id) {
                combo_box->setCurrentIndex(combo_box->count() - 1);
            }
        }
        right_layout->addWidget(combo_box);
        QObject::connect(combo_box, &QComboBox::currentIndexChanged, [key, layout_ids](int index) {
            if (index == 0) {
                Application::instance().getSettingsManager().resetSetting(key);
                return;
            }
            Application::instance().getSettingsManager().setLayoutSetting(key, layout_ids[index - 1]);
        });
        return wrapInSettingsWidget(right_layout);
    }
} // namespace magnesia
