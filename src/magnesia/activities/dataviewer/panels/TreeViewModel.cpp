#include "activities/dataviewer/panels/TreeViewModel.hpp"

#include "opcua_qt/abstraction/node/Node.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    using opcua_qt::abstraction::Node;

    TreeViewModel::TreeViewModel(QObject* parent) : QAbstractItemModel(parent), m_root_node(nullptr) {}

    void TreeViewModel::setRootNode(Node* root) {
        beginResetModel();
        m_root_node = root;
        endResetModel();
    }

    QModelIndex TreeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (m_root_node == nullptr || row < 0 || column < 0) {
            return {};
        }

        auto* parent_node = getNode(parent);

        // Start tree with root node
        if (parent_node == nullptr) {
            parent_node = m_root_node;
        }

        auto* node = parent_node->getChildren().value(row);
        return node != nullptr ? createIndex(row, column, node) : QModelIndex();
    }

    QModelIndex TreeViewModel::parent(const QModelIndex& index) const {
        if (!index.isValid()) {
            return {};
        }

        auto* node = getNode(index);

        // Root does not have a parent
        if (node == m_root_node) {
            return {};
        }

        auto* parent_node = node->getParent();
        if (parent_node == m_root_node) {
            // Root always has index 0
            return createIndex(0, 0, parent_node);
        }

        const int row = getChildIndexOf(parent_node->getParent(), parent_node);
        return createIndex(row, 0, parent_node);
    }

    int TreeViewModel::rowCount(const QModelIndex& parent) const {
        if (parent.column() > 0) {
            return 0;
        }

        auto* node = getNode(parent);
        if (node == nullptr) {
            node = m_root_node;
        }

        return static_cast<int>(node->getChildren().size());
    }

    int TreeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 1;
    }

    QVariant TreeViewModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        return getNode(index)->getDisplayName().getText();
    }

    QVariant TreeViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            if (section == 0) {
                return "Name";
            }
        }
        return {};
    }

    int TreeViewModel::getChildIndexOf(Node* parent, Node* child) {
        auto children = parent->getChildren();

        for (int i = 0; i < children.size(); i++) {
            if (children.value(i) == child) {
                return i;
            }
        }

        return -1;
    }

    Node* TreeViewModel::getNode(const QModelIndex& index) {
        return static_cast<Node*>(index.internalPointer());
    }
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
