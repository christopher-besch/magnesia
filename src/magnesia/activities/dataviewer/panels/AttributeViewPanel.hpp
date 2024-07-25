#pragma once

#include "activities/dataviewer/Panel.hpp"
#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/dataviewer_fwd.hpp"
#include "activities/dataviewer/panels/AttributeViewModel.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "opcua_qt/abstraction/node/Node.hpp"

#include <QLabel>
#include <QList>
#include <QTreeView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {

    class AttributeViewPanel : public Panel {

      public:
        explicit AttributeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

        [[nodiscard]] const PanelMetadata& metadata() const noexcept override;

      signals:
        void nodeSelected(const opcua_qt::abstraction::NodeId& node_id);
        void nodeSelectedRecursive(const opcua_qt::abstraction::NodeId& node_id);

      public slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node_id) override;

      private:
        QList<opcua_qt::abstraction::Node> m_nodes;
        QTreeView*                         m_tree_view;
        AttributeViewModel*                m_model;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"attributeview",
        .name   = u"AttributeView",
        .create = create_helper<AttributeViewPanel>,
    };

} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel

#pragma once
