#pragma once

#include "opcua_qt/abstraction/node/Node.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    class TreeViewModel : public QAbstractItemModel {
        Q_OBJECT

      public:
        explicit TreeViewModel(QObject* parent = nullptr);

        [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;
        [[nodiscard]] int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int         columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;

        void setRootNode(opcua_qt::abstraction::Node* root);

        [[nodiscard]] static magnesia::opcua_qt::abstraction::Node* nodeForIndex(const QModelIndex& index);

      private:
        [[nodiscard]] static int getChildIndexOf(opcua_qt::abstraction::Node* parent,
                                                 opcua_qt::abstraction::Node* child);

        [[nodiscard]] static opcua_qt::abstraction::Node* getNode(const QModelIndex& index);

      private:
        opcua_qt::abstraction::Node* m_root_node;
    };
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
