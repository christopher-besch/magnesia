#pragma once

#include "../../Activity.hpp"
#include "../../ActivityMetadata.hpp"
#include "../../Application.hpp"

#include <QWidget>

namespace magnesia::activities::add_activity {
    /**
     * @class AddActivity
     * @brief The entry activity in the GUI for the user.
     *
     * Provides a list of activities the user can create and the framework
     * to configure them. All the activity-specific details are provided by the activity developers through the
     * respective `ActivityMetadata` instance.
     *
     * @see Activity
     * @see ActivityMetadata
     */
    class AddActivity : public Activity {
      public:
        /**
         * @param parent Parent of the AddActivity.
         */
        explicit AddActivity(QWidget* parent = nullptr);
    };

    inline constexpr ActivityMetadata metadata{
        .name        = u"Add Activity",
        .global_init = [] { Application::instance().openActivity(new AddActivity, "+", false); },
    };
} // namespace magnesia::activities::add_activity
