#include "AttributeViewPanel.hpp"

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../panels.hpp"
#include "AttributeViewModel.hpp"

#include <optional>

#include <QDebug>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    using opcua_qt::abstraction::NodeId;

    AttributeViewPanel::AttributeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, PanelType::attributeview, attribute_view_panel::metadata, parent),
          m_tree_view(new QTreeView(this)), m_model(new AttributeViewModel(this)) {
        m_tree_view->setModel(m_model);
        m_tree_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_tree_view->setFrameShape(QFrame::Shape::NoFrame);

        auto* layout = new QHBoxLayout;
        layout->addWidget(m_tree_view);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);

        setLayout(layout);
    }

    void AttributeViewPanel::selectNode(const NodeId& node_id) {
        auto* connection = getDataViewer()->getConnection();
        if (auto node = connection->getNode(node_id); node.has_value()) {
            m_model->setNode(*node, connection);
        }
    }
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
