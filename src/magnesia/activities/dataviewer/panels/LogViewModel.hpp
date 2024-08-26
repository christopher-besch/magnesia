#pragma once

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/abstraction/LogCategory.hpp"

#include <vector>

#include <QAbstractTableModel>
#include <QObject>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    /**
     * @class LogViewModel
     * @brief Model for the LogViewPanel.
     */
    class LogViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        /**
         * @param parent Parent of the LogViewModel.
         */
        explicit LogViewModel(QObject* parent = nullptr);

        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;

        /**
         * Loads log lines inside the view.
         *
         * @param log_lines Log lines of the node.
         */
        void setLogLines(std::vector<opcua_qt::LogEntry>& log_lines);

        /**
         * Saves the log to a file.
         *
         * @param file_name Name of the file.
         * @return whether or not saving was successful.
         */
        bool saveLogToFile(const QString& file_name);

        /**
         * Retrieves the log lines of the view.
         *
         * @return List containing the log entries of the node.
         */
        [[nodiscard]] std::vector<opcua_qt::LogEntry> getLogLines() const;

      public slots:

        /**
         * Adds a Log line into the view.
         *
         * @param entry Log entry to be added.
         */
        void addLogLine(const opcua_qt::LogEntry& entry);

        /**
         * Clears the logs in the view.
         */
        void clearLogs();

      private:
        std::vector<opcua_qt::LogEntry> m_log_lines;
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
