#include "activities/dataviewer/panels/ReferenceViewPanel.hpp"

#include "activities/dataviewer/DataViewer.hpp"
#include "activities/dataviewer/Panel.hpp"
#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/panels.hpp"
#include "activities/dataviewer/panels/ReferenceViewModel.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {

    ReferenceViewPanel::ReferenceViewPanel(DataViewer* data_viewer, QWidget* parent)
        : Panel(data_viewer, Panels::reference_view, parent),
          m_reference_view_model(new ReferenceViewModel(data_viewer, this)), m_table_view(new QTableView(this)),
          m_data_viewer(data_viewer) {

        m_table_view->setModel(m_reference_view_model);
        m_table_view->horizontalHeader()->setStretchLastSection(true);
        m_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        // Vertical layout for the entire panel
        auto* toplayout = new QVBoxLayout(this);
        toplayout->addWidget(m_table_view); // Then add the table view
    }

    void ReferenceViewPanel::selectNode(const opcua_qt::abstraction::NodeId& node_id) {
        auto* node = m_data_viewer->getConnection()->getNode(node_id);
        m_reference_view_model->nodeSelected(node);
    }

    const PanelMetadata& ReferenceViewPanel::metadata() const noexcept {
        return reference_view_panel::metadata;
    };
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
