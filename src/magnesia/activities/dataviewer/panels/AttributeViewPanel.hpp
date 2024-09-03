#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "AttributeViewModel.hpp"

#include <QTreeView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {

    /**
     * @class AttributeViewPanel
     * @brief Panel displaying the attributes of a node.
     */
    class AttributeViewPanel : public Panel {
      public:
        /**
         * @param dataviewer Dataviewer in which the panel is embedded.
         * @param parent Parent of the panel.
         */
        explicit AttributeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      private slots:
        void selectNode(const opcua_qt::abstraction::NodeId& node_id) override;

      private:
        QTreeView*          m_tree_view;
        AttributeViewModel* m_model;
    };

    inline constexpr PanelMetadata metadata{
        .id     = u"attributeview",
        .name   = u"AttributeView",
        .create = create_helper<AttributeViewPanel>,
    };
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
