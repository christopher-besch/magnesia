#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "TreeViewModel.hpp"

#include <QModelIndex>
#include <QTreeView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    /**
     * @class TreeViewPanel
     * @brief Panel for displaying nodes in a tree structure.
     */
    class TreeViewPanel : public Panel {
        Q_OBJECT

      public:
        /**
         * @param dataviewer DataViewer
         * @param parent Parent of the panel.
         */
        explicit TreeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      private:
        void indexSelected(QModelIndex index, panels::PanelTypes recipients);

      signals:
        /**
         * Emitted when a node is selected.
         *
         * @param node Id of the node.
         * @param recipients Panels receiving the selection.
         */
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients);

      private:
        QTreeView*     m_tree_view;
        TreeViewModel* m_model;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"treeview",
        .name   = u"TreeView",
        .create = create_helper<TreeViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
