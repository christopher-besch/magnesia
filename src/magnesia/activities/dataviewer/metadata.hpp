#pragma once

#include "../../ActivityMetadata.hpp"
#include "../../ConfigWidget.hpp"
#include "../../qt_version_check.hpp"
#include "ConfigWidget.hpp"

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer {
    inline constexpr ActivityMetadata metadata{
        .name                 = u"DataViewer",
        .global_init          = []() { qDebug() << "Initialized DataViewer"; },
        .create_config_widget = []() -> magnesia::ConfigWidget* { return new ConfigWidget; },
    };
} // namespace magnesia::activities::dataviewer
