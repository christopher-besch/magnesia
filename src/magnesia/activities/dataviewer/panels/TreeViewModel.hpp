#pragma once

#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::treeview_panel {
    /**
     * @class TreeViewModel
     * @brief Model for the TreeViewPanel.
     */
    class TreeViewModel : public QAbstractItemModel {
        Q_OBJECT

      public:
        /**
         * @param parent Parent of the model.
         */
        explicit TreeViewModel(QObject* parent = nullptr);

        [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;
        [[nodiscard]] int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int         columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant    headerData(int section, Qt::Orientation orientation,
                                             int role = Qt::DisplayRole) const override;
        [[nodiscard]] bool        canFetchMore(const QModelIndex& parent) const override;
        [[nodiscard]] bool        hasChildren(const QModelIndex& parent) const override;

        /**
         * Retrieves the node in the treeview.
         * @param index Index inside the view.
         * @return Node inside the treeview.
         */
        [[nodiscard]] static opcua_qt::abstraction::Node* getNode(const QModelIndex& index);

        /**
         * Sets the root node inside the TreeViewPanel.
         * @param root new root node of the panel.
         */
        void setRootNode(opcua_qt::abstraction::Node* root);

      private:
        [[nodiscard]] static int getChildIndexOf(opcua_qt::abstraction::Node* parent,
                                                 opcua_qt::abstraction::Node* child);

      private:
        opcua_qt::abstraction::Node* m_root_node{nullptr};
    };
} // namespace magnesia::activities::dataviewer::panels::treeview_panel
