#pragma once

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <utility>
#include <vector>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {
    /**
     * @class ReferenceViewModel
     * @brief Model for the ReferenceViewPanel.
     */
    class ReferenceViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        /**
         * @param connection Connection to the Node.
         * @param parent Parent of the model.
         */
        explicit ReferenceViewModel(opcua_qt::Connection* connection, QObject* parent = nullptr);

        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;
        /**
         * Selects a node inside the ReferenceViewModel.
         *
         * @param node Node
         */
        void nodeSelected(opcua_qt::abstraction::Node* node);

      private:
        opcua_qt::Connection*                    m_connection;
        std::vector<std::pair<QString, QString>> m_references;
    };
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
