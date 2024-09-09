#pragma once

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/AttributeId.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    /**
     * @class AttributeViewModel
     * @brief Model for AttributeViewPanel.
     */
    class AttributeViewModel : public QAbstractItemModel {
        Q_OBJECT

      public:
        /**
         * @param parent Parent of the AttributeViewModel.
         */
        explicit AttributeViewModel(QObject* parent = nullptr);

        [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QModelIndex parent(const QModelIndex& index) const override;
        [[nodiscard]] int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int         columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant    headerData(int section, Qt::Orientation orientation,
                                             int role = Qt::DisplayRole) const override;

        /**
         * Set the view to a specific Node.
         *
         * @param node The node.
         * @param connection Connection to the node.
         */
        void setNode(opcua_qt::abstraction::Node* node, opcua_qt::Connection* connection);

      private slots:
        void valueChanged(opcua_qt::abstraction::Node* node, opcua_qt::abstraction::AttributeId attribute_id);

      private:
        std::vector<opcua_qt::abstraction::AttributeId> m_available_attributes;
        opcua_qt::abstraction::Node*                    m_node{nullptr};
        opcua_qt::Connection*                           m_connection{nullptr};
        opcua_qt::abstraction::Subscription*            m_subscription{nullptr};
    };
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
