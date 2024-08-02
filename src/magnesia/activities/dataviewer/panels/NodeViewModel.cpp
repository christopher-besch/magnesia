#include "NodeViewModel.hpp"

#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../DataViewer.hpp"

#include <QAbstractTableModel>
#include <QList>
#include <QObject>
#include <QStack>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::node_view_panel {

    NodeViewModel::NodeViewModel(DataViewer* data_viewer, QObject* parent)
        : QAbstractTableModel(parent), m_data_viewer(data_viewer) {}

    int NodeViewModel::rowCount(const QModelIndex& /*parent*/) const {
        return static_cast<int>(m_nodes.size());
    }

    int NodeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        constexpr int eight = 8;
        return eight;
    }

    QVariant NodeViewModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        auto* node = m_nodes.at(index.row());
        if (node == nullptr) {
            return {};
        }

        constexpr int case_5 = 5;
        constexpr int case_6 = 6;
        constexpr int case_7 = 7;
        switch (index.column()) {
            case 0:
                return index.row();
            case 1:
                return node->getNodeId().toString();
            case 2:
                return node->getDisplayName().getText();
            default:
                break;
        }

        auto data_value = node->getDataValue();
        if (!data_value.has_value()) {
            return {};
        }

        switch (index.column()) {
            case 3:
                return data_value->getValue().toString();
            case 4:
                return data_value->getDataTypeName();
            case case_5:
                return data_value->getSourceTimestamp();
            case case_6:
                return data_value->getServerTimestamp();
            case case_7:
                return data_value->getStatusCode().get();
            default:
                return {};
        }
    }

    QVariant NodeViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole) {
            return {};
        }
        constexpr int case_5 = 5;
        constexpr int case_6 = 6;
        constexpr int case_7 = 7;

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString("#");
                case 1:
                    return QString("Node Id");
                case 2:
                    return QString("Display Name");
                case 3:
                    return QString("Value");
                case 4:
                    return QString("Datatype");
                case case_5:
                    return QString("Source Timestamp");
                case case_6:
                    return QString("Server Timestamp");
                case case_7:
                    return QString("Statuscode");
                default:
                    return {};
            }
        }
        return {};
    }

    void NodeViewModel::nodeSelected(opcua_qt::abstraction::Node* node) {
        beginResetModel();

        m_nodes.clear();
        if (node != nullptr) {
            QList<opcua_qt::abstraction::Node*> leaf_nodes;
            findLeafNodes(node, leaf_nodes);
            m_nodes.append(leaf_nodes);
        }
        endResetModel();
    }

    void NodeViewModel::findLeafNodes(opcua_qt::abstraction::Node*         node,
                                      QList<opcua_qt::abstraction::Node*>& leaf_nodes) {
        if (node == nullptr) {
            return;
        }

        QStack<opcua_qt::abstraction::Node*> stack;
        stack.push(node);

        while (!stack.isEmpty()) {
            auto* current  = stack.pop();
            auto  children = current->getChildren();

            if (children.isEmpty()) {
                leaf_nodes.append(current);
            } else {
                for (const auto& child : children) {
                    stack.push(child);
                }
            }
        }
    }

} // namespace magnesia::activities::dataviewer::panels::node_view_panel
