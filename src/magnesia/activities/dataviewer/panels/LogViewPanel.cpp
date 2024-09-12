#include "LogViewPanel.hpp"

#include "../../../opcua_qt/abstraction/LogLevel.hpp"
#include "../../../qt_version_check.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../panels.hpp"
#include "LogViewModel.hpp"

#include <QAbstractItemView>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringBuilder>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtTypeTraits>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::panels::log_view_panel {
    LogViewPanel::LogViewPanel(DataViewer* dataviewer, QWidget* parent)
        : Panel(dataviewer, PanelType::logview, log_view_panel::metadata, parent), m_table(new QTableView),
          m_level_selector(new QComboBox) {
        auto* layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);

        auto* tool_layout = new QHBoxLayout;
        layout->addLayout(tool_layout);

        m_table->setFrameShape(QFrame::Shape::NoFrame);

        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_table->horizontalHeader()->setStretchLastSection(true);
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        auto* model = new LogViewModel(dataviewer->getLogger(), this);

        auto* filter_model = new LogFilterModel(this);
        filter_model->setSourceModel(model);
        filter_model->setMinLevel(opcua_qt::LogLevel::DEBUG);

        m_table->setModel(filter_model);

        layout->addWidget(m_table);

        auto* export_button = new QPushButton("Export");
        connect(export_button, &QPushButton::clicked, this, &LogViewPanel::exportLog);
        tool_layout->addWidget(export_button);

        auto* clear_button = new QPushButton("Clear");
        connect(clear_button, &QPushButton::clicked, this,
                [model, filter_model] { filter_model->setMinimumRow(model->rowCount()); });
        tool_layout->addWidget(clear_button);

        connect(m_level_selector, &QComboBox::currentIndexChanged, filter_model, [this, filter_model](int index) {
            auto new_level = m_level_selector->itemData(index).value<opcua_qt::LogLevel>();
            filter_model->setMinLevel(new_level);
        });

        for (auto level : {
                 opcua_qt::LogLevel::TRACE,
                 opcua_qt::LogLevel::DEBUG,
                 opcua_qt::LogLevel::INFO,
                 opcua_qt::LogLevel::WARNING,
                 opcua_qt::LogLevel::ERROR,
                 opcua_qt::LogLevel::FATAL,
             }) {
            m_level_selector->addItem(opcua_qt::log_level_to_string(level), QVariant::fromValue(level));
        }
        // TODO: setCurrentText or setCurrentIndex?
        m_level_selector->setCurrentText(opcua_qt::log_level_to_string(opcua_qt::LogLevel::INFO));
        tool_layout->addWidget(m_level_selector);

        setLayout(layout);
    }

    void LogViewPanel::exportLog() {
        auto*     model   = m_table->model();
        const int rows    = model->rowCount();
        const int columns = model->columnCount();

        // cppcheck doesn't like Q_ASSERT(columns > 0) or assert(columns > 0)
        if (columns < 1) {
            Q_ASSERT(false);
            return;
        }

        QString result;

        for (int column = 0; column < columns; column++) {
            result += model->headerData(column, Qt::Horizontal).toString() % ',';
        }
        result[result.size() - 1] = '\n';
        for (int row = 0; row < rows; row++) {
            for (int column = 0; column < columns; column++) {
                auto column_data = model->data(model->index(row, column)).toString();
                if (column_data.contains(',')) {
                    result += '"' % column_data.replace('\\', "\\\\").replace('"', "\\\"") % "\",";
                } else {
                    result += column_data % ',';
                }
            }
            result[result.size() - 1] = '\n';
        }

        auto url = QFileDialog::getSaveFileUrl(this, "Save Log", {}, "Log Files (*.csv)");
        if (url.isEmpty()) {
            return;
        }
        QFile file{url.toLocalFile()};
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::critical(this, "Export failed", "Couldn't open file for writing", QMessageBox::Close);
        }
        file.write(result.toUtf8());
    }

    QJsonObject LogViewPanel::saveState() const {
        return {
            {"level", qToUnderlying(m_level_selector->currentData().value<opcua_qt::LogLevel>())},
        };
    }

    bool LogViewPanel::restoreState(const QJsonObject& state) {
        auto level = state["level"];
        if (!level.isDouble()) {
            return false;
        }

        m_level_selector->setCurrentText(opcua_qt::log_level_to_string(static_cast<opcua_qt::LogLevel>(level.toInt())));
        return true;
    }
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
