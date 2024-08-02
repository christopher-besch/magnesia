#pragma once

#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../dataviewer_fwd.hpp"

#include <QAbstractTableModel>
#include <QList>
#include <QObject>
#include <QVariant>
#include <QWidget>
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

        void nodeSelected(opcua_qt::abstraction::Node* node);

      private:
        static void findLeafNodes(opcua_qt::abstraction::Node* node, QList<opcua_qt::abstraction::Node*>& leaf_nodes);

        DataViewer*                         m_data_viewer;
        QList<opcua_qt::abstraction::Node*> m_nodes;
    };
} // namespace magnesia::activities::dataviewer::panels::node_view_panel
