#pragma once

#include "../../../opcua_qt/Logger.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QModelIndex>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    /**
     * @class LogViewModel
     * @brief Model for the LogViewPanel
     */
    class LogViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        explicit LogViewModel(opcua_qt::Logger* logger, QObject* parent = nullptr);

        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;

      private:
        friend class LogFilterModel;

        enum {
            LevelColumn,
            CategoryColumn,
            MessageColumn,

            COLUMN_COUNT,
        };

      private:
        opcua_qt::Logger* m_logger;
    };

    /**
     * @class LogFilterModel
     * @brief Proxy model to enable the LogViewPanel to filter log entries
     */
    class LogFilterModel : public QSortFilterProxyModel {
        Q_OBJECT

      public:
        using QSortFilterProxyModel::QSortFilterProxyModel;

        [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /**
         * Set minimum log level used to filter rows
         *
         * @param min_level the new minimum log level
         */
        void setMinLevel(opcua_qt::LogLevel min_level);
        /**
         * Set minimum log entry row. Used to clear a panel without clearing underlying data.
         *
         * @param row the new minimum log entry row
         */
        void setMinimumRow(int row);

      private:
        opcua_qt::LogLevel m_min_level{};
        int                m_min_row{};
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
