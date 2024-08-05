#pragma once

#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../dataviewer_fwd.hpp"

#include <utility>

#include <QAbstractTableModel>
#include <QList>
#include <QObject>
#include <QVariant>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {

    class ReferenceViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        explicit ReferenceViewModel(DataViewer* data_viewer, QObject* parent = nullptr);
        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;
        void                   nodeSelected(opcua_qt::abstraction::Node* node);

      private:
        DataViewer*                        m_data_viewer;
        QList<std::pair<QString, QString>> m_references;
    };

} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
