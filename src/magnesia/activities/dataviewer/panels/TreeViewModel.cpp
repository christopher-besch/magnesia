#include "TreeViewModel.hpp"

#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    using opcua_qt::abstraction::Node;

    TreeViewModel::TreeViewModel(QObject* parent) : QAbstractItemModel(parent) {}

    void TreeViewModel::setRootNode(Node* root) {
        beginResetModel();
        m_root_node = root;
        endResetModel();
    }

    QModelIndex TreeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (m_root_node == nullptr || !checkIndex(parent)) {
            return {};
        }

        auto* parent_node = getNode(parent);
        Node* node        = nullptr;

        // Start tree with root node
        if (parent_node == nullptr) {
            node = m_root_node;
        } else {
            node = parent_node->getChildren().value(row);
        }

        return node != nullptr ? createIndex(row, column, node) : QModelIndex();
    }

    QModelIndex TreeViewModel::parent(const QModelIndex& index) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
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
        if (!checkIndex(parent)) {
            return 0;
        }

        auto* node = getNode(parent);
        if (node == nullptr) {
            return 1;
        }

        return static_cast<int>(node->getChildren().size());
    }

    int TreeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 1;
    }

    QVariant TreeViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::DisplayRole) {
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

    bool TreeViewModel::canFetchMore(const QModelIndex& parent) const {
        const auto* node = getNode(parent);
        if (node == nullptr) {
            return true;
        }
        return !node->childrenCountCached().has_value();
    }

    bool TreeViewModel::hasChildren(const QModelIndex& parent) const {
        const auto* node = getNode(parent);
        if (node == nullptr) {
            return true;
        }

        auto children_count = node->childrenCountCached();
        if (!children_count.has_value()) {
            // we don't yet know if the node has children
            // return true to enable finding out later
            return true;
        }
        return children_count.value() != 0;
    }
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
