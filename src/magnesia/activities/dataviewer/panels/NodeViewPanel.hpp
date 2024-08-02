#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "NodeViewModel.hpp"

#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::node_view_panel {
    class NodeViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit NodeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

        [[nodiscard]] const PanelMetadata& metadata() const noexcept override;

      public slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node) override;

      private:
        NodeViewModel* m_node_view_model;
        QTableView*    m_table_view;
        DataViewer*    m_data_viewer;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"nodeview",
        .name   = u"NodeView",
        .create = [](DataViewer* dataviewer) -> Panel* { return new NodeViewPanel(dataviewer); },
    };
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
