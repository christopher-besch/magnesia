#include "activities/dataviewer/panels/AttributeViewPanel.hpp"

#include "Application.hpp"
#include "activities/dataviewer/DataViewer.hpp"
#include "activities/dataviewer/Panel.hpp"
#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/panels.hpp"
#include "activities/dataviewer/panels/AttributeViewModel.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    using opcua_qt::abstraction::NodeId;

    AttributeViewPanel::AttributeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, Panels::attribute, parent), m_tree_view(new QTreeView(this)),
          m_model(new AttributeViewModel(this)) {

        m_tree_view->setModel(m_model);
        m_tree_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

        auto* layout = new QHBoxLayout;
        layout->addWidget(m_tree_view);
        layout->setAlignment(Qt::AlignCenter);
        setLayout(layout);
    }

    void AttributeViewPanel::selectNode(const NodeId& node_id) {
        // TODO: Get real id
        const int connection_id = 1;
        auto*     connection    = Application::instance().getConnectionManager().getConnection(connection_id);
        m_model->setNode(connection->getNode(node_id), connection);
    }

    const PanelMetadata& AttributeViewPanel::metadata() const noexcept {
        return attribute_view_panel::metadata;
    };
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
