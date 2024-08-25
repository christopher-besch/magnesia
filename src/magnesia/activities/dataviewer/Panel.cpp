#include "Panel.hpp"

#include "../../opcua_qt/abstraction/NodeId.hpp"
#include "DataViewer.hpp"
#include "PanelMetadata.hpp"
#include "dataviewer_fwd.hpp"
#include "panels.hpp"

#include <QJsonObject>
#include <QWidget>

namespace magnesia::activities::dataviewer {
    Panel::Panel(DataViewer* dataviewer, panels::PanelType panel, PanelMetadata metadata, QWidget* parent)
        : QWidget(parent), m_panel_type(panel), m_metadata(metadata), m_dataviewer(dataviewer) {
        connect(m_dataviewer, &DataViewer::nodeSelected, this, &Panel::selectNodeAll);
    }

    void Panel::selectNodeAll(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients) {
        if (recipients.testFlag(m_panel_type)) {
            selectNode(node);
        }
    }

    void Panel::selectNode(const opcua_qt::abstraction::NodeId& /*node*/) {
        // don't do anything in the default implementation
    }

    QJsonObject Panel::saveState() const {
        // don't do anything in the default implementation
        return {};
    };

    bool Panel::restoreState(const QJsonObject& /*data*/) {
        // don't do anything in the default implementation
        return true;
    }

    const PanelMetadata& Panel::metadata() const noexcept {
        return m_metadata;
    }

    DataViewer* Panel::getDataViewer() const noexcept {
        return m_dataviewer;
    }
} // namespace magnesia::activities::dataviewer
