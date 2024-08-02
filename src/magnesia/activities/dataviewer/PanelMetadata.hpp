#pragma once

#include "dataviewer_fwd.hpp"

#include <QStringView>
#include <QWidget>

namespace magnesia::activities::dataviewer {
    struct PanelMetadata {
        /**
         * Internal id used for layout (de-)serialization. Must be unique across the lifetime of persisted layouts (also
         * across program versions)
         */
        QStringView id;

        /**
         * Displayed to the user in panel type selection
         */
        QStringView name;

        /**
         * Called with DataViewer instance to create a panel instance.
         */
        Panel* (*create)(DataViewer*){};
    };

    template<typename PanelType>
    constexpr Panel* create_helper(DataViewer* dataviewer) {
        return new PanelType(dataviewer);
    };
} // namespace magnesia::activities::dataviewer
