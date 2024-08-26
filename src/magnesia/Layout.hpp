#pragma once

#include <QJsonDocument>
#include <QString>

namespace magnesia {
    /**
     * @class Layout
     * @brief The layout of panels inside a tab.
     */
    struct Layout {
        ///  Name of the layout.
        QString name;
        /// JSON Document containing the layout.
        QJsonDocument json_data;
    };
} // namespace magnesia
