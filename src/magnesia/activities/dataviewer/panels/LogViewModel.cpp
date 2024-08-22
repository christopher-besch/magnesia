#include "LogViewModel.hpp"

#include "../../../opcua_qt/LogEntry.hpp"
#include "../../../opcua_qt/abstraction/LogCategory.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"

#include <cstddef>
#include <vector>

#include <QAbstractTableModel>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVariant>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    LogViewModel::LogViewModel(QObject* parent) : QAbstractTableModel(parent) {}

    int LogViewModel::rowCount(const QModelIndex& /*parent*/) const {
        return static_cast<int>(m_log_lines.size());
    }

    int LogViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 3;
    }

    QVariant LogViewModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        const auto& log_line = m_log_lines[static_cast<std::size_t>(index.row())];
        switch (index.column()) {
            case 0:
                return opcua_qt::log_level_to_string(log_line.getLevel());
            case 1:
                return opcua_qt::log_category_to_string(log_line.getCategory());
            case 2:
                return log_line.getMessage();
            default:
                return {};
        }
    }

    QVariant LogViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole) {
            return {};
        }

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString("Log Level");
                case 1:
                    return QString("Log Category");
                case 2:
                    return QString("Log Message");
                default:
                    return {};
            }
        }
        return {};
    }

    void LogViewModel::setLogLines(const std::vector<opcua_qt::LogEntry>& log_lines) {
        beginResetModel();
        m_log_lines = log_lines;
        endResetModel();
    }

    void LogViewModel::addLogLine(const opcua_qt::LogEntry& entry) {
        auto size = static_cast<int>(m_log_lines.size());
        beginInsertRows({}, size, size);
        m_log_lines.push_back(entry);
        endInsertRows();
    }

    void LogViewModel::clearLogs() {
        beginResetModel();
        m_log_lines.clear();
        endResetModel();
    }

    bool LogViewModel::saveLogToFile(const QString& file_name) {
        QFile file(file_name);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream out(&file);

        // Write headers
        out << "Level,Category,Message\n";

        for (const auto& log_line : m_log_lines) {
            const QString level    = opcua_qt::log_level_to_string(log_line.getLevel());
            const QString category = opcua_qt::log_category_to_string(log_line.getCategory());
            QString       message  = log_line.getMessage();

            out << level << "," << category << "," << message.replace(",", " ") << "\n";
        }

        file.close();
        return true;
    }

    const std::vector<opcua_qt::LogEntry>& LogViewModel::getLogLines() const {
        return m_log_lines;
    }
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
