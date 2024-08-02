#pragma once

#include <QJsonDocument>
#include <QString>

namespace magnesia {
    /**
     * The layout of panels inside a tab.
     */
    struct Layout {
        QString       name;
        QJsonDocument json_data;
    };
} // namespace magnesia
