#pragma once

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "../panels/LogViewModel.hpp"

#include <vector>

#include <QComboBox>
#include <QDebug>
#include <QObject>
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    /**
     * @class LogViewPanel
     * @brief Panel for displaying the log of a node.
     */
    class LogViewPanel : public Panel {
        Q_OBJECT

      public:
        /**
         * @param dataviewer Dataviewer in which the panel is embedded.
         * @param parent Parent of the panel.
         */
        explicit LogViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      private slots:
        /**
         * Changes the log level.
         *
         * @param level New log level.
         */
        void changedLogLevel(opcua_qt::LogLevel level);

        /**
         * Adds a log entry.
         *
         * @param entry Log entry.
         */
        void log(const opcua_qt::LogEntry& entry);

        /**
         * Saves the log.
         */
        void saveLog();

        /**
         * Filters the log entries to the current log level.
         */
        void filterLogs();

      private:
        opcua_qt::LogLevel m_current_log_level{magnesia::opcua_qt::LogLevel::DEBUG};
        LogViewModel*      m_log_view_model;
        QTableView*        m_table_view;
        QPushButton*       m_clear_log_button;
        QPushButton*       m_save_log_button;
        QComboBox*         m_log_level_combo_box;

        std::vector<opcua_qt::LogEntry> m_log_lines;
        std::vector<opcua_qt::LogEntry> m_filtered_log_lines;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"logview",
        .name   = u"LogView",
        .create = create_helper<LogViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
