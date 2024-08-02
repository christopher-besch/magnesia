#pragma once

#include "activities/dataviewer/Panel.hpp"
#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/dataviewer_fwd.hpp"
#include "activities/dataviewer/panels/ReferenceViewModel.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {

    class ReferenceViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit ReferenceViewPanel(DataViewer* data_viewer, QWidget* parent = nullptr);

        [[nodiscard]] const PanelMetadata& metadata() const noexcept override;

      public slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node) override;

      private:
        ReferenceViewModel* m_reference_view_model;
        QTableView*         m_table_view;
        DataViewer*         m_data_viewer;
    };

    inline constexpr magnesia::activities::dataviewer::PanelMetadata metadata{
        .id     = u"referenceview",
        .name   = u"ReferenceView",
        .create = create_helper<ReferenceViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
