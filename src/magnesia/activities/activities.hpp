#pragma once

#include "../ActivityMetadata.hpp"

#include <array>

namespace magnesia::activities {
    /**
     * List of registered activities. This is mainly used by `Application::initializeActivites()` and the AddActivity.
     * Add an Activity's ActivityMetadata here to initialize it at application startup and/or make it show up for the
     * user to create.
     */
    inline constexpr std::array<ActivityMetadata, 0> all{};
} // namespace magnesia::activities
