#include "Panel.hpp"

#include "activities/dataviewer/DataViewer.hpp"
#include "activities/dataviewer/panels.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QWidget>

namespace magnesia::activities::dataviewer {
    Panel::Panel(DataViewer* dataviewer, panels::Panels panel, QWidget* parent) : QWidget(parent), m_panel_type(panel) {
        connect(dataviewer, &DataViewer::nodeSelected, this, &Panel::selectNodeAll);
    }

    void Panel::selectNodeAll(const opcua_qt::abstraction::NodeId& node, panels::Panels recipients) {
        if ((recipients & m_panel_type) == m_panel_type) {
            selectNode(node);
        }
    }

    void Panel::selectNode(const opcua_qt::abstraction::NodeId& /*node*/) {
        // don't do anything in the default implementation
    }
} // namespace magnesia::activities::dataviewer
