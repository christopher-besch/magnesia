#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "NodeViewModel.hpp"

#include <QModelIndex>
#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::node_view_panel {
    /**
     * @class NodeViewPanel
     * @brief Panel for displaying nodes.
     */
    class NodeViewPanel : public Panel {
        Q_OBJECT

      public:
        /**
         * @param dataviewer DataViewer displaying the panel.
         * @param parent Parent of the panel.
         */
        explicit NodeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      private slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node) override;
        void onCurrentNodeChanged(const QModelIndex& current);

      private:
        NodeViewModel* m_model;
        QTableView*    m_table_view;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"nodeview",
        .name   = u"NodeView",
        .create = create_helper<NodeViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
