#include "LogViewModel.hpp"

#include "../../../opcua_qt/Logger.hpp"
#include "../../../opcua_qt/abstraction/LogCategory.hpp"
#include "../../../opcua_qt/abstraction/LogLevel.hpp"
#include "../../../qt_version_check.hpp"

#include <cstddef>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    LogViewModel::LogViewModel(opcua_qt::Logger* logger, QObject* parent)
        : QAbstractTableModel(parent), m_logger(logger) {
        Q_ASSERT(m_logger != nullptr);

        connect(logger, &opcua_qt::Logger::logEntryAdded, this, [this](std::size_t index) {
            auto row = static_cast<int>(index);
            // TODO: How broken is this? This adds the entry before calling beginInsertRows.
            beginInsertRows({}, row, row);
            endInsertRows();
        });
    }

    int LogViewModel::rowCount(const QModelIndex& /*parent*/) const {
        return static_cast<int>(m_logger->getEntries().size());
    }

    int LogViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return COLUMN_COUNT;
    }

    QVariant LogViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
            return {};
        }

        auto        row   = static_cast<std::size_t>(index.row());
        const auto& entry = m_logger->getEntries()[row];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case LevelColumn:
                    return opcua_qt::log_level_to_string(entry.getLevel());
                case CategoryColumn:
                    return opcua_qt::log_category_to_string(entry.getCategory());
                case MessageColumn:
                    return entry.getMessage();
                default:
                    Q_ASSERT(false);
            }
        }

        if (role == Qt::UserRole) {
            switch (index.column()) {
                case LevelColumn:
                    return QVariant::fromValue(entry.getLevel());
                case CategoryColumn:
                    return QVariant::fromValue(entry.getCategory());
                case MessageColumn:
                    return entry.getMessage();
                default:
                    Q_ASSERT(false);
            }
        }

        return {};
    }

    QVariant LogViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole) {
            return {};
        }

        if (orientation != Qt::Horizontal) {
            return QAbstractTableModel::headerData(section, orientation, role);
        }

        switch (section) {
            case LevelColumn:
                return "Level";
            case CategoryColumn:
                return "Category";
            case MessageColumn:
                return "Message";
            default:
                Q_ASSERT(false);
        }

        return {};
    }

    bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
        if (source_row < m_min_row) {
            return false;
        }
        auto index = sourceModel()->index(source_row, LogViewModel::LevelColumn, source_parent);
        auto level = sourceModel()->data(index, Qt::UserRole).value<opcua_qt::LogLevel>();
        return level >= m_min_level;
    }

    void LogFilterModel::setMinLevel(opcua_qt::LogLevel min_level) {
        m_min_level = min_level;
        invalidateFilter();
    }

    void LogFilterModel::setMinimumRow(int row) {
        m_min_row = row;
        invalidateFilter();
    }
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
