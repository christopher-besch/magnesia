#include "NodeViewPanel.hpp"

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../dataviewer_fwd.hpp"
#include "../panels.hpp"
#include "../panels/NodeViewModel.hpp"

#include <algorithm>

#include <QAbstractItemView>
#include <QFrame>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QtEvents>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::node_view_panel {
    NodeViewPanel::NodeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, PanelType::nodeview, node_view_panel::metadata, parent),
          m_model(new NodeViewModel(dataviewer, this)), m_table_view(new QTableView) {
        m_table_view->setModel(m_model);
        m_table_view->horizontalHeader()->setStretchLastSection(true);
        m_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table_view->setSelectionMode(QAbstractItemView::ContiguousSelection);
        m_table_view->setFrameShape(QFrame::Shape::NoFrame);

        auto* layout = new QVBoxLayout;
        layout->addWidget(m_table_view);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);

        connect(m_table_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
                &NodeViewPanel::onCurrentNodeChanged);

        m_table_view->setContextMenuPolicy(Qt::ActionsContextMenu);
        auto* remove = m_table_view->addAction("Remove selected", Qt::Key_Delete, this, [this] {
            if (!m_table_view->selectionModel()->hasSelection()) {
                return;
            }

            auto rows  = m_table_view->selectionModel()->selectedRows();
            auto start = std::ranges::min_element(rows, {}, &QModelIndex::row)->row();
            auto count = static_cast<int>(rows.size());

            m_model->removeRows(start, count);
        });
        remove->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }

    void NodeViewPanel::onCurrentNodeChanged(const QModelIndex& current) {
        const auto* node = m_model->getNode(current);
        if (node == nullptr) {
            return;
        }

        Q_EMIT nodeSelected(node->getNodeId(), PanelType::attributeview | PanelType::referenceview);
    }

    void NodeViewPanel::selectNode(const opcua_qt::abstraction::NodeId& node_id) {
        auto* connection = getDataViewer()->getConnection();
        auto* node       = connection->getNode(node_id);
        m_model->appendNode(node, connection);
    }
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
