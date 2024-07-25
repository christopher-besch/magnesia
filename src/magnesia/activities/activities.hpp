#pragma once

#include "add_activity/AddActivity.hpp"
#include "dataviewer/DataViewer.hpp"

#include <array>

namespace magnesia::activities {
    /**
     * List of registered activities. This is mainly used by `Application::initializeActivites()` and the AddActivity.
     * Add an Activity's ActivityMetadata here to initialize it at application startup and/or make it show up for the
     * user to create.
     */
    inline constexpr std::array all{
        dataviewer::metadata,

        // NOTE: Keep add_activity last. It calls metadata.create_config_widget which may depend on other activities
        // already being initialized
        add_activity::metadata,
    };
} // namespace magnesia::activities
