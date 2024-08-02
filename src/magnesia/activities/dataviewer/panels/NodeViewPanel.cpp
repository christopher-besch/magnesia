#include "NodeViewPanel.hpp"

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "../panels.hpp"
#include "../panels/NodeViewModel.hpp"

#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::node_view_panel {

    NodeViewPanel::NodeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, Panels::node, parent), m_node_view_model(new NodeViewModel(dataviewer, this)),
          m_table_view(new QTableView(this)), m_data_viewer(dataviewer) {
        m_table_view->setModel(m_node_view_model);
        m_table_view->horizontalHeader()->setStretchLastSection(true);
        m_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        auto* layout = new QVBoxLayout;
        layout->addWidget(m_table_view);
        setLayout(layout);
    }

    void NodeViewPanel::selectNode(const opcua_qt::abstraction::NodeId& node_id) {
        auto* node = m_data_viewer->getConnection()->getNode(node_id);
        m_node_view_model->nodeSelected(node);
    }

    const PanelMetadata& NodeViewPanel::metadata() const noexcept {
        return node_view_panel::metadata;
    };
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
