#pragma once

#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"

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
     * @class LogViewPanel
     * @brief Panel for displaying the log of the current dataviewer instance.
     */
    class LogViewPanel : public Panel {
        Q_OBJECT

      public:
        /**
         * @param dataviewer Dataviewer in which the panel is embedded.
         * @param parent Qt parent of the panel.
         */
        explicit LogViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

        [[nodiscard]] QJsonObject saveState() const override;
        [[nodiscard]] bool        restoreState(const QJsonObject& state) override;

      private slots:
        void exportLog();

      private:
        QTableView* m_table{};
        QComboBox*  m_level_selector{};
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"logview",
        .name   = u"LogView",
        .create = create_helper<LogViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::log_view_panel
