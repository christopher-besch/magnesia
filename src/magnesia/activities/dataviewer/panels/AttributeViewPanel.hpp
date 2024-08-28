#pragma once

#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../Panel.hpp"
#include "../PanelMetadata.hpp"
#include "../dataviewer_fwd.hpp"
#include "AttributeViewModel.hpp"

#include <QLabel>
#include <QTreeView>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {

    class AttributeViewPanel : public Panel {
      public:
        explicit AttributeViewPanel(DataViewer* dataviewer, QWidget* parent = nullptr);

      signals:
        void nodeSelected(const opcua_qt::abstraction::NodeId& node_id);
        void nodeSelectedRecursive(const opcua_qt::abstraction::NodeId& node_id);

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

#pragma once
