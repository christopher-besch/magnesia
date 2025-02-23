#include "TreeViewModel.hpp"

#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    using opcua_qt::abstraction::Node;

    TreeViewModel::TreeViewModel(QObject* parent) : QAbstractItemModel(parent) {}

    QModelIndex TreeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (m_root_node == nullptr || !checkIndex(parent)) {
            return {};
        }

        auto* parent_node = getNode(parent);
        Node* node        = nullptr;

        // Start tree with root node
        if (parent_node == nullptr) {
            node = m_root_node;
        } else if (const auto* children = parent_node->getChildren(); children != nullptr) {
            node = (*children)[static_cast<std::size_t>(row)];
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
        const auto* children = node->getChildren();
        if (children == nullptr) {
            return 0;
        }
        return static_cast<int>(children->size());
    }

    int TreeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 1;
    }

    QVariant TreeViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::DisplayRole) {
            return {};
        }

        if (const auto* display_name = getNode(index)->getDisplayName(); display_name != nullptr) {
            return display_name->getText();
        }
        return {};
    }

    QVariant TreeViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            if (section == 0) {
                return "Name";
            }
        }
        return {};
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

    void TreeViewModel::setRootNode(Node* root) {
        beginResetModel();
        m_root_node = root;
        endResetModel();
    }

    Node* TreeViewModel::getNode(const QModelIndex& index) {
        return static_cast<Node*>(index.internalPointer());
    }

    int TreeViewModel::getChildIndexOf(Node* parent, Node* child) {
        if (const auto* children = parent->getChildren(); children != nullptr) {
            auto iter = std::ranges::find(*children, child);
            return iter != children->end() ? static_cast<int>(std::distance(children->begin(), iter)) : -1;
        }
        return -1;
    }

} // namespace magnesia::activities::dataviewer::panels::treeview_panel
