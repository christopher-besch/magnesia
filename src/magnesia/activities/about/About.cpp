#include "About.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../ConfigWidget.hpp"
#include "../../qt_version_check.hpp"

#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QResource>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::about {
    About::About(QWidget* parent) : Activity(parent) {
        auto* layout = new QVBoxLayout();
        auto* title  = new QLabel("Magnesia");
        QFont font   = title->font();
        font.setPointSize(s_title_size);
        title->setFont(font);
        auto* version_header = new QLabel("Version:");
        font                 = version_header->font();
        font.setPointSize(s_header_size);
        version_header->setFont(font);

        auto* license_header = new QLabel("Application license:");
        license_header->setFont(font);

        auto*           version_label = new QLabel("0.0.1-pre-alpha");
        const QResource license{":/licenses/LICENSE"};
        Q_ASSERT(license.isValid());
        auto* license_text_edit = new QTextEdit();
        license_text_edit->setText(license.uncompressedData());
        license_text_edit->setReadOnly(true);

        layout->addWidget(title);
        layout->addWidget(version_header);
        layout->addWidget(version_label);

        layout->addWidget(license_header);
        layout->addWidget(license_text_edit);
        layout->setAlignment(Qt::AlignCenter);

        setLayout(layout);
    }

    ConfigWidget::ConfigWidget(QWidget* parent) : magnesia::ConfigWidget(parent) {
        auto* layout = new QVBoxLayout;

        auto* create_button = new QPushButton("Open About");
        connect(create_button, &QPushButton::clicked, this, &ConfigWidget::create);
        layout->addWidget(create_button);

        setLayout(layout);
    }

    // slots apparently can't be static in Qt 6.4
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void ConfigWidget::create() {
        Application::instance().openActivity(new About, "About");
    }
} // namespace magnesia::activities::about
