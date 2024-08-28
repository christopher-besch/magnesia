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
    class NodeViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit NodeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      signals:
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients);

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
