#include "AddActivity.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"

#include <functional>

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QStackedLayout>
#include <QWidget>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::add_activity {
    AddActivity::AddActivity(QWidget* parent) : Activity{parent} {
        auto* h_layout = new QHBoxLayout;

        auto* activity_list       = new QListWidget;
        auto* config_widget_stack = new QStackedLayout;

        for (const auto& activity : Application::getActivityMetadata()) {
            if (activity.create_config_widget != nullptr) {
                activity_list->addItem(activity.name.toString());
                config_widget_stack->addWidget(std::invoke(activity.create_config_widget));
            }
        }

        connect(activity_list, &QListWidget::currentRowChanged, config_widget_stack, &QStackedLayout::setCurrentIndex);

        auto* fallback_label  = new QLabel("Select an activity in the list to start");
        auto* fallback_layout = new QHBoxLayout;
        fallback_layout->addWidget(fallback_label, 0, Qt::AlignCenter);
        auto* fallback_widget = new QWidget;
        fallback_widget->setLayout(fallback_layout);

        auto index = config_widget_stack->addWidget(fallback_widget);
        config_widget_stack->setCurrentIndex(index);

        h_layout->addWidget(activity_list, 1);
        h_layout->addLayout(config_widget_stack, 5); // NOLINT: cppcoreguidelines-avoid-magic-numbers

        setLayout(h_layout);
    }

    void AddActivity::closeEvent(QCloseEvent* event) {
        Q_ASSERT(event != nullptr);
        // This Activity cannot be closed, it is the main entry point for the user.
        qDebug() << "AddActivity: ignoring CloseEvent";
        event->ignore();
    }
} // namespace magnesia::activities::add_activity
