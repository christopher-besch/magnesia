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
    class TreeViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit TreeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

        [[nodiscard]] const PanelMetadata& metadata() const noexcept override;

      private:
        void indexSelected(QModelIndex index, panels::PanelTypes recipients);

      signals:
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
