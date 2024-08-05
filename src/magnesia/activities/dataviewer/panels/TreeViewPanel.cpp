
#include "TreeViewPanel.hpp"

#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../panels.hpp"
#include "TreeViewModel.hpp"

#include <QDebug>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QObject>
#include <QTreeView>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    using opcua_qt::abstraction::Node;

    TreeViewPanel::TreeViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, Panels::treeview, parent), m_tree_view(new QTreeView(this)),
          m_model(new TreeViewModel(this)) {

        auto* connection = dataviewer->getConnection();
        auto* root_node  = connection->getRootNode();

        m_model->setRootNode(root_node);
        m_tree_view->setModel(m_model);

        auto* layout = new QHBoxLayout;
        layout->addWidget(m_tree_view);
        layout->setAlignment(Qt::AlignCenter);
        setLayout(layout);

        connect(m_tree_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
                &TreeViewPanel::onCurrentNodeChanged);
        connect(this, &TreeViewPanel::nodeSelected, dataviewer, &DataViewer::nodeSelected);
    }

    void TreeViewPanel::onCurrentNodeChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
        auto* node = static_cast<Node*>(current.internalPointer());

        if (node == nullptr) {
            return;
        }

        Q_EMIT nodeSelected(node->getNodeId(), Panels::attribute | Panels::reference_view);
    }

    const PanelMetadata& TreeViewPanel::metadata() const noexcept {
        return treeview_panel::metadata;
    }
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
