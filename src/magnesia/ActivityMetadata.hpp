#pragma once

#include "ConfigWidget.hpp"
#include <QStringView>

namespace magnesia {
    /**
     * Metadata struct for activities. Every activity needs an instance of this struct. It defines what code to run on
     * application startup and what `ConfigWidget` to display in the AddActivity Activity. These are the entry points
     * available to activity developers.
     *
     * @see Activity
     * @see ConfigWidget
     */
    struct ActivityMetadata {
        /**
         * The display name of the activity. Shown in the AddActivity Activity when `createConfigWidget` is not
         * `nullptr`.
         */
        QStringView name;

        /**
         * Function that is called on application startup. Can, among other things, be used to register settings with
         * the `SettingsManager`, register `URLHandler` or create an instance of the activity. Can be `nullptr`.
         */
        void (*global_init)(){};

        /**
         * Function that is called by the AddActivity Activity once to create the `ConfigWidget` for the activity. Set
         * to `nullptr` to hide the activity from the AddActivity Activity.
         */
        ConfigWidget* (*create_config_widget)(){};
    };
} // namespace magnesia
