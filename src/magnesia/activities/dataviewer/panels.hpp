#pragma once

#include "dataviewer_fwd.hpp"
#include "panels/AttributeViewPanel.hpp"
#include "panels/LogViewPanel.hpp"
#include "panels/NodeViewPanel.hpp"
#include "panels/ReferenceViewPanel.hpp"
#include "panels/TreeViewPanel.hpp"

#include <array>

namespace magnesia::activities::dataviewer::panels {
    /**
     * List of registered panels used to fill the UI. Add your panels metadata here and an entry to `panels::PanelType`.
     */
    inline constexpr std::array all{
        log_view_panel::metadata,       treeview_panel::metadata,  attribute_view_panel::metadata,
        reference_view_panel::metadata, node_view_panel::metadata,
    };

    /**
     * Bitset used internally for identifying panel types at runtime. Add your panels here and it's metadata to
     * `panels::all`.
     */
    enum class PanelType : unsigned {
        treeview       = 0x1 << 1,
        attribute      = 0x1 << 2,
        log_view       = 0x1 << 3,
        reference_view = 0x1 << 4,
        node           = 0x1 << 5,
    };
} // namespace magnesia::activities::dataviewer::panels
