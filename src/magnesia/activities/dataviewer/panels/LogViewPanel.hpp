#pragma once

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "../panels/LogViewModel.hpp"

#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QObject>
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    class LogViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit LogViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

        [[nodiscard]] const PanelMetadata& metadata() const noexcept override;

      public slots:
        void changedLogLevel(opcua_qt::LogLevel level);
        void log(const opcua_qt::LogEntry& entry);
        void saveLog();
        void filterLogs();

      private:
        opcua_qt::LogLevel m_current_log_level;
        LogViewModel*      m_log_view_model;
        QTableView*        m_table_view;
        QPushButton*       m_clear_log_button;
        QPushButton*       m_save_log_button;
        QComboBox*         m_log_level_combo_box;

        QList<opcua_qt::LogEntry> m_log_lines;
        QList<opcua_qt::LogEntry> m_filtered_log_lines;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"logview",
        .name   = u"LogView",
        .create = create_helper<LogViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
