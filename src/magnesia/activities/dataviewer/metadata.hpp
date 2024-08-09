#pragma once

#include "../../ActivityMetadata.hpp"
#include "../../ConfigWidget.hpp"
#include "ConfigWidget.hpp"

namespace magnesia::activities::dataviewer {
    inline constexpr ActivityMetadata metadata{
        .name                 = u"DataViewer",
        .create_config_widget = []() -> magnesia::ConfigWidget* { return new ConfigWidget; },
    };
} // namespace magnesia::activities::dataviewer
