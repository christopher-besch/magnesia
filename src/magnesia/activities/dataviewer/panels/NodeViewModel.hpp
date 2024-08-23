#pragma once

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/Subscription.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../dataviewer_fwd.hpp"

#include <vector>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::node_view_panel {
    class NodeViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        explicit NodeViewModel(DataViewer* data_viewer, QObject* parent = nullptr);

        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;
        bool                   removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

        void appendNode(opcua_qt::abstraction::Node* node, opcua_qt::Connection* connection);
        [[nodiscard]] opcua_qt::abstraction::Node* getNode(QModelIndex index) const;

      private:
        static std::vector<opcua_qt::abstraction::Node*> findLeafNodes(opcua_qt::abstraction::Node* node);
        void subscribeNodes(const std::vector<opcua_qt::abstraction::Node*>& nodes, opcua_qt::Connection* connection);

      private:
        DataViewer*                                       m_data_viewer;
        std::vector<opcua_qt::abstraction::Node*>         m_nodes;
        std::vector<opcua_qt::abstraction::Subscription*> m_subscriptions;

      private:
        enum {
            NodeIdColumn,
            DisplayNameColumn,
            ValueColumn,
            DataTypeColumn,
            SourceTimestampColumn,
            ServerTimestampColumn,
            StatusCodeColumn,

            COLUMN_COUNT,
        };
    };
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
