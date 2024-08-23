#pragma once

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/abstraction/LogCategory.hpp"

#include <vector>

#include <QAbstractTableModel>
#include <QObject>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    class LogViewModel : public QAbstractTableModel {
        Q_OBJECT

      public:
        explicit LogViewModel(QObject* parent = nullptr);

        [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                          int role = Qt::DisplayRole) const override;

        void                                          setLogLines(std::vector<opcua_qt::LogEntry>& log_lines);
        bool                                          saveLogToFile(const QString& file_name);
        [[nodiscard]] std::vector<opcua_qt::LogEntry> getLogLines() const;

      public slots:
        void addLogLine(const opcua_qt::LogEntry& entry);
        void clearLogs();

      private:
        std::vector<opcua_qt::LogEntry> m_log_lines;
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
