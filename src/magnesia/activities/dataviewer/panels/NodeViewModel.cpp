#include "NodeViewModel.hpp"

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/AttributeId.hpp"
#include "../../../opcua_qt/abstraction/DataValue.hpp"
#include "../../../opcua_qt/abstraction/LocalizedText.hpp"
#include "../../../opcua_qt/abstraction/NodeClass.hpp"
#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../../../opcua_qt/abstraction/Subscription.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../../../qt_version_check.hpp"
#include "../DataViewer.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QStack>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtAlgorithms>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::panels::node_view_panel {
    using opcua_qt::Connection;
    using opcua_qt::abstraction::AttributeId;
    using opcua_qt::abstraction::DataValue;
    using opcua_qt::abstraction::Node;
    using opcua_qt::abstraction::NodeClass;
    using opcua_qt::abstraction::Subscription;

    NodeViewModel::NodeViewModel(DataViewer* data_viewer, QObject* parent)
        : QAbstractTableModel(parent), m_data_viewer(data_viewer) {}

    int NodeViewModel::rowCount(const QModelIndex& /*parent*/) const {
        return static_cast<int>(m_nodes.size());
    }

    int NodeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return COLUMN_COUNT;
    }

    QVariant NodeViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::DisplayRole) {
            return {};
        }

        auto* node = getNode(index);

        switch (index.column()) {
            case NodeIdColumn:
                return node->getNodeId().toString();
            case DisplayNameColumn:
                return node->getDisplayName().getText();
            default:
                // Other case checked below
                break;
        }

        auto data_value = node->getDataValue();
        if (!data_value.has_value()) {
            return {};
        }

        switch (index.column()) {
            case ValueColumn:
                return data_value->getValue().toString();
            case DataTypeColumn:
                return data_value->getDataTypeName();
            case SourceTimestampColumn:
                return data_value->getSourceTimestamp();
            case ServerTimestampColumn:
                return data_value->getServerTimestamp();
            case StatusCodeColumn:
                return data_value->getStatusCode().toString();
            default:
                return {};
        }
    }

    QVariant NodeViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole) {
            return {};
        }

        if (orientation != Qt::Horizontal) {
            return QAbstractTableModel::headerData(section, orientation, role);
        }

        switch (section) {
            case NodeIdColumn:
                return "Node Id";
            case DisplayNameColumn:
                return "Display Name";
            case ValueColumn:
                return "Value";
            case DataTypeColumn:
                return "Datatype";
            case SourceTimestampColumn:
                return "Source Timestamp";
            case ServerTimestampColumn:
                return "Server Timestamp";
            case StatusCodeColumn:
                return "Statuscode";
            default:
                return {};
        }

        return {};
    }

    void NodeViewModel::appendNode(Node* node, Connection* connection) {
        beginResetModel();

        if (node != nullptr) {
            auto leaf_nodes = findLeafNodes(node);
            subscribeNodes(leaf_nodes, connection);
            m_nodes.insert(m_nodes.end(), leaf_nodes.begin(), leaf_nodes.end());
        }

        endResetModel();
    }

    bool NodeViewModel::removeRows(int row, int count, const QModelIndex& parent) {
        beginRemoveRows(parent, row, row + count - 1);

        auto nodes_first = m_nodes.begin() + row;
        auto nodes_last  = nodes_first + count;
        qDeleteAll(nodes_first, nodes_last);
        m_nodes.erase(nodes_first, nodes_last);
        m_subscriptions.erase(m_subscriptions.begin() + row, m_subscriptions.begin() + row + count);

        endRemoveRows();

        return true;
    }

    std::vector<Node*> NodeViewModel::findLeafNodes(Node* node) {
        if (node == nullptr) {
            return {};
        }

        std::vector<Node*> leaf_nodes;
        QStack<Node*>      stack;
        stack.push(node);

        while (!stack.isEmpty()) {
            auto*      current  = stack.pop();
            const auto children = current->getChildren();

            for (const auto& child : children) {
                stack.push(child);
            }

            if (current->getNodeClass() == NodeClass::VARIABLE
                || (current->getNodeClass() == NodeClass::VARIABLE_TYPE && current->getDataValue().has_value())) {
                leaf_nodes.push_back(current);
            }
        }

        return leaf_nodes;
    }

    void NodeViewModel::subscribeNodes(std::span<Node*> nodes, Connection* connection) {
        const std::vector attribute_ids{
            AttributeId::DISPLAY_NAME,
            AttributeId::VALUE,
        };

        for (auto* node : nodes) {
            auto* subscription = connection->createSubscription(node, attribute_ids);
            connect(
                subscription, &Subscription::valueChanged, this,
                [&](Node* subscribed_node, AttributeId /*attribute_id*/, const std::shared_ptr<DataValue>& /*value*/) {
                    auto node_it = std::ranges::find(m_nodes, subscribed_node);
                    Q_ASSERT(node_it != m_nodes.cend());
                    auto row = static_cast<int>(std::distance(m_nodes.begin(), node_it));

                    auto left_index  = createIndex(row, 0);
                    auto right_index = createIndex(row, COLUMN_COUNT - 1); // -1 because both indices are inclusive
                    Q_EMIT dataChanged(left_index, right_index, {Qt::DisplayRole});
                });
            subscription->setPublishingMode(true);

            m_subscriptions.push_back(subscription);
        }
    }

    opcua_qt::abstraction::Node* NodeViewModel::getNode(QModelIndex index) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
            return nullptr;
        }

        return m_nodes[static_cast<std::size_t>(index.row())];
    }

} // namespace magnesia::activities::dataviewer::panels::node_view_panel
