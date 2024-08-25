#pragma once

#include <QFlags>

namespace magnesia::activities::dataviewer {
    class DataViewer;
    class Panel;

    namespace layout {
        class PanelLayout;
    } // namespace layout

    namespace panels {
        enum class PanelType : unsigned;
        Q_DECLARE_FLAGS(PanelTypes, PanelType)
        Q_DECLARE_OPERATORS_FOR_FLAGS(PanelTypes)
    } // namespace panels
} // namespace magnesia::activities::dataviewer
