#pragma once

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "Application.hpp"
#include "qt_version_check.hpp"

#include <QCloseEvent>
#include <QWidget>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::add_activity {
    /**
     * The main entry point in the GUI for the user. Provides a list of activities the user can create and the framework
     * to configure them. All the activity-specific details are provided by the activity developers through the
     * respective `ActivityMetadata` instance.
     *
     * @see Activity
     * @see ActivityMetadata
     */
    class AddActivity : public Activity {
      public:
        explicit AddActivity(QWidget* parent = nullptr);

        void closeEvent(QCloseEvent* event) override;
    };

    inline constexpr ActivityMetadata metadata{
        .name = u"Add Activity",
        .global_init =
            []() {
                Application::instance().openActivity(new AddActivity, "+");
                qDebug() << "Initialized AddActivity";
            },
    };
} // namespace magnesia::activities::add_activity
