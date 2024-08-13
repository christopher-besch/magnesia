#include "Panel.hpp"

#include "../../opcua_qt/abstraction/NodeId.hpp"
#include "DataViewer.hpp"
#include "panels.hpp"

#include <QJsonObject>
#include <QWidget>

namespace magnesia::activities::dataviewer {
    Panel::Panel(DataViewer* dataviewer, panels::Panels panel, QWidget* parent)
        : QWidget(parent), m_panel_type(panel), m_dataviewer(dataviewer) {
        connect(m_dataviewer, &DataViewer::nodeSelected, this, &Panel::selectNodeAll);
    }

    void Panel::selectNodeAll(const opcua_qt::abstraction::NodeId& node, panels::Panels recipients) {
        if ((recipients & m_panel_type) == m_panel_type) {
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

    DataViewer* Panel::getDataViewer() const noexcept {
        return m_dataviewer;
    }
} // namespace magnesia::activities::dataviewer
