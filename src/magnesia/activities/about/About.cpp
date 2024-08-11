#include "About.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../ConfigWidget.hpp"
#include "../../qt_version_check.hpp"
#include "version.hpp"

#include <QFont>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResource>
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
        layout->setAlignment(Qt::AlignCenter);

        {
            auto* title = new QLabel("Magnesia");

            auto font = title->font();
            font.setPointSize(s_title_size);
            title->setFont(font);

            layout->addWidget(title);
        }

        {
            auto* version_header = new QLabel("Version:");

            auto font = version_header->font();
            font.setPointSize(s_header_size);
            version_header->setFont(font);

            layout->addWidget(version_header);
        }
        {
            auto* version_label = new QLabel{cmake::project_version.data()};
            layout->addWidget(version_label);
        }

        {
            auto* license_header = new QLabel("Application license:");

            auto font = license_header->font();
            font.setPointSize(s_header_size);
            license_header->setFont(font);

            layout->addWidget(license_header);
        }
        {
            auto* license_text_edit = new QPlainTextEdit();

            license_text_edit->setReadOnly(true);
            license_text_edit->setLineWrapMode(QPlainTextEdit::NoWrap);

            const QResource license{":/licenses/LICENSE"};
            Q_ASSERT(license.isValid());
            license_text_edit->setPlainText(license.uncompressedData());

            auto font = license_text_edit->document()->defaultFont();
            font.setFamily("monospace");
            font.setStyleHint(QFont::Monospace);
            license_text_edit->document()->setDefaultFont(font);

            layout->addWidget(license_text_edit);
        }

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
