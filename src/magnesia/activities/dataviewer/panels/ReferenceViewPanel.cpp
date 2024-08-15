#include "ReferenceViewPanel.hpp"

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../DataViewer.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../panels.hpp"
#include "ReferenceViewModel.hpp"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

namespace magnesia::activities::dataviewer::panels::reference_view_panel {
    ReferenceViewPanel::ReferenceViewPanel(DataViewer* data_viewer, QWidget* parent)
        : Panel(data_viewer, Panels::reference_view, parent),
          m_reference_view_model(new ReferenceViewModel(data_viewer->getConnection(), this)),
          m_table_view(new QTableView) {

        m_table_view->setModel(m_reference_view_model);
        m_table_view->horizontalHeader()->setStretchLastSection(true);
        m_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_table_view->setFrameShape(QFrame::Shape::NoFrame);

        auto* toplayout = new QVBoxLayout;
        toplayout->addWidget(m_table_view);
        toplayout->setContentsMargins(0, 0, 0, 0);

        setLayout(toplayout);
    }

    void ReferenceViewPanel::selectNode(const opcua_qt::abstraction::NodeId& node_id) {
        auto* node = getDataViewer()->getConnection()->getNode(node_id);
        m_reference_view_model->nodeSelected(node);
    }

    const PanelMetadata& ReferenceViewPanel::metadata() const noexcept {
        return reference_view_panel::metadata;
    };
} // namespace magnesia::activities::dataviewer::panels::reference_view_panel
