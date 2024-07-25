#include "DataViewer.hpp"

#include "Activity.hpp"
#include "Application.hpp"
#include "ConfigWidget.hpp"
#include "layout.hpp"

#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer {
    DataViewer::DataViewer(QWidget* parent) : Activity(parent) {
        auto* layout       = new QVBoxLayout;
        auto* panel_layout = new layout::PanelLayout(this, Qt::Horizontal, nullptr);

        layout->addWidget(panel_layout);
        panel_layout->addWidget();

        setLayout(layout);
    }

    ConfigWidget::ConfigWidget(QWidget* parent) : magnesia::ConfigWidget(parent) {
        auto* layout = new QVBoxLayout;

        // TODO: build connection when ConnectionManager is available
        auto* create_button = new QPushButton("Create");
        connect(create_button, &QPushButton::clicked, this, &ConfigWidget::create);
        layout->addWidget(create_button);

        setLayout(layout);
    }

    void ConfigWidget::create() {
        Application::instance().openActivity(new DataViewer, "DataViewer " + QString::number(++m_count));
    }
} // namespace magnesia::activities::dataviewer
