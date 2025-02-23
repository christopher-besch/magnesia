#include "TreeViewPanel.hpp"

#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../dataviewer_fwd.hpp"
#include "../panels.hpp"
#include "TreeViewModel.hpp"

#include <optional>

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QObject>
#include <QTreeView>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    TreeViewPanel::TreeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, PanelType::treeview, treeview_panel::metadata, parent), m_tree_view(new QTreeView(this)),
          m_model(new TreeViewModel(this)) {
        auto* layout = new QHBoxLayout;
        layout->addWidget(m_tree_view);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);

        auto* connection = dataviewer->getConnection();
        auto  root_node  = connection->getRootNode();
        if (!root_node.has_value()) {
            return;
        }
        m_model->setRootNode(*root_node);
        m_tree_view->setModel(m_model);
        m_tree_view->setFrameShape(QFrame::Shape::NoFrame);
        m_tree_view->setExpandsOnDoubleClick(false);

        connect(m_tree_view, &QTreeView::clicked, this, [this](QModelIndex index) {
            indexSelected(index, PanelType::attributeview | PanelType::referenceview);
        });

        connect(m_tree_view, &QTreeView::doubleClicked, this,
                [this](QModelIndex index) { indexSelected(index, PanelType::nodeview); });
    }

    void TreeViewPanel::indexSelected(QModelIndex index, panels::PanelTypes recipients) {
        const auto* node = TreeViewModel::getNode(index);
        if (node == nullptr) {
            return;
        }

        Q_EMIT nodeSelected(node->getNodeId(), recipients);
    }
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
