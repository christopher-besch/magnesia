#include "LogViewPanel.hpp"

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/Logger.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../panels.hpp"
#include "LogViewModel.hpp"

#include <QBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QTableView>
#include <QVariant>
#include <QWidget>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    LogViewPanel::LogViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, Panels::log_view, parent), m_current_log_level(magnesia::opcua_qt::LogLevel::DEBUG),
          m_log_view_model(new LogViewModel(this)), m_table_view(new QTableView(this)),
          m_log_level_combo_box(new QComboBox(this)) {

        m_table_view->setModel(m_log_view_model);
        m_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table_view->horizontalHeader()->setStretchLastSection(true);
        m_table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        m_clear_log_button = new QPushButton("Clear Log", this);
        m_save_log_button  = new QPushButton("Save Log", this);

        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::TRACE),
                                       QVariant::fromValue(opcua_qt::LogLevel::TRACE));
        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::DEBUG),
                                       QVariant::fromValue(opcua_qt::LogLevel::DEBUG));
        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::INFO),
                                       QVariant::fromValue(opcua_qt::LogLevel::INFO));
        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::WARNING),
                                       QVariant::fromValue(opcua_qt::LogLevel::WARNING));
        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::ERROR),
                                       QVariant::fromValue(opcua_qt::LogLevel::ERROR));
        m_log_level_combo_box->addItem(opcua_qt::log_level_to_string(opcua_qt::LogLevel::FATAL),
                                       QVariant::fromValue(opcua_qt::LogLevel::FATAL));

        // Horizontal layout for buttons and combo box
        auto* button_layout = new QHBoxLayout();
        button_layout->addWidget(m_clear_log_button);
        button_layout->addWidget(m_save_log_button);
        button_layout->addWidget(m_log_level_combo_box);

        // Vertical layout for the entire panel
        auto* toplayout = new QVBoxLayout(this);
        toplayout->addLayout(button_layout); // Add the horizontal layout first
        toplayout->addWidget(m_table_view);  // Then add the table view

        connect(dataviewer->getLogger(), &opcua_qt::Logger::logEntryAdded, this, &LogViewPanel::log);
        connect(m_clear_log_button, &QPushButton::clicked, m_log_view_model, &LogViewModel::clearLogs);
        connect(m_save_log_button, &QPushButton::clicked, this, &LogViewPanel::saveLog);
        connect(m_log_level_combo_box, &QComboBox::currentIndexChanged, [this](int /*index*/) {
            auto level = m_log_level_combo_box->currentData().value<opcua_qt::LogLevel>();
            this->changedLogLevel(level);
        });

        setLayout(toplayout);
        auto old_logs = dataviewer->getLogger()->getLogForLevel({
            opcua_qt::LogLevel::TRACE,
            opcua_qt::LogLevel::INFO,
            opcua_qt::LogLevel::DEBUG,
            opcua_qt::LogLevel::ERROR,
            opcua_qt::LogLevel::FATAL,
            opcua_qt::LogLevel::WARNING,
        });

        for (const auto& entry : old_logs) {
            this->log(entry);
        }

        filterLogs();
    }

    void LogViewPanel::changedLogLevel(opcua_qt::LogLevel level) {
        m_current_log_level = level;
        filterLogs();
    }

    void LogViewPanel::log(const opcua_qt::LogEntry& entry) {
        if (entry.getLevel() >= m_current_log_level) {
            m_log_view_model->addLogLine(entry);
        }
        m_log_lines.append(entry);
    }

    void LogViewPanel::saveLog() {
        const QString file_name = QFileDialog::getSaveFileName(this, "Save Log", "", "Log Files (*.csv)");
        if (!file_name.isEmpty()) {
            if (!m_log_view_model->saveLogToFile(file_name)) {
                QMessageBox::warning(this, "Save Log", "Failed to save log to file.");
            }
        }
    }

    void LogViewPanel::filterLogs() {
        QList<opcua_qt::LogEntry> filtered_logs;
        for (const auto& log_line : m_log_lines) {
            if (log_line.getLevel() >= m_current_log_level) {
                filtered_logs.append(log_line);
            }
        }
        m_filtered_log_lines = filtered_logs;
        m_log_view_model->setLogLines(m_filtered_log_lines);
    }

    const PanelMetadata& LogViewPanel::metadata() const noexcept {
        return log_view_panel::metadata;
    }
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
