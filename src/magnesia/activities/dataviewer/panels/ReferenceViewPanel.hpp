#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "ReferenceViewModel.hpp"

#include <QTableView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {
    class ReferenceViewPanel : public Panel {
        Q_OBJECT

      public:
        explicit ReferenceViewPanel(DataViewer* data_viewer, QWidget* parent = nullptr);

      private slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node) override;

      private:
        ReferenceViewModel* m_reference_view_model;
        QTableView*         m_table_view;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"referenceview",
        .name   = u"ReferenceView",
        .create = create_helper<ReferenceViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
