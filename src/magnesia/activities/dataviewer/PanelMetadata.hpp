#pragma once

#include "Panel.hpp"
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
        Panel* (*create)(DataViewer*){};
    };

    template<typename PanelType>
    constexpr Panel* create_helper(DataViewer* dataviewer) {
        return new PanelType(dataviewer);
    };
} // namespace magnesia::activities::dataviewer
