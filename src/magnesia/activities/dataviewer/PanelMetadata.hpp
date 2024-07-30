#pragma once

#include "dataviewer_fwd.hpp"

#include <QStringView>
#include <QWidget>

namespace magnesia::activities::dataviewer {
    struct PanelMetadata {
        /**
         * Displayed to the user in panel type selection
         */
        QStringView name;

        /**
         * Called with dataviewer instance to create a panel instance.
         */
        QWidget* (*create)(DataViewer*){};
    };
} // namespace magnesia::activities::dataviewer
