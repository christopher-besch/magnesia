#include "AddActivity.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"

#include <functional>

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLoggingCategory>
#include <QStackedLayout>
#include <QWidget>
#include <Qt>

namespace {
    Q_LOGGING_CATEGORY(lc_add_activity, "magnesia.addactivity")
} // namespace

namespace magnesia::activities::add_activity {
    AddActivity::AddActivity(QWidget* parent) : Activity{parent} {
        auto* h_layout = new QHBoxLayout;

        auto* activity_list       = new QListWidget;
        auto* config_widget_stack = new QStackedLayout;

        for (const auto& activity : Application::getActivityMetadata()) {
            if (activity.create_config_widget != nullptr) {
                qCInfo(lc_add_activity) << "adding activity" << activity.name;
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
        config_widget_stack->setContentsMargins(0, 0, 0, 0);

        h_layout->addWidget(activity_list, 1);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        h_layout->addLayout(config_widget_stack, 5);
        h_layout->setContentsMargins(4, 4, 4, 4);

        setLayout(h_layout);
    }
} // namespace magnesia::activities::add_activity
