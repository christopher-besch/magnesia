#pragma once

#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/dataviewer_fwd.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QJsonObject>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    /**
     * Base class for implementing panels for the `DataViewer`. Provides an abstraction for inter-panel communication.
     */
    class Panel : public QWidget {
        Q_OBJECT

      public:
        [[nodiscard]] virtual QJsonObject saveState() const;
        [[nodiscard]] virtual bool        restoreState(const QJsonObject& data);

        [[nodiscard]] virtual const PanelMetadata& metadata() const noexcept = 0;

      protected:
        /**
         * @param dataviewer the `DataViewer` this panel belongs to.
         * @param panel the type of this panel. Requires an entry in the `panels::Panels` enum.
         * @param parent the parent QWidget passed to Qt.
         */
        explicit Panel(DataViewer* dataviewer, panels::Panels panel, QWidget* parent = nullptr);

      protected slots:
        /**
         * Called when any Panel signals that a node was selected. The default implementation filters calls to the ones
         * targeted at this panels type and passes them to `Panel::selectNode`. You should not need to override this
         * function directly, see `Panel::selectNode` instead.
         *
         * @param node the selected node's id.
         * @param recipients the types of panels that are targeted by this call.
         */
        virtual void selectNodeAll(const opcua_qt::abstraction::NodeId& node, panels::Panels recipients);
        /**
         * Called when any Panel signals that a node was selected and targets this panel's type. The default
         * implementation is a no-op.
         *
         * @param node the selected node's id.
         */
        virtual void selectNode(const opcua_qt::abstraction::NodeId& node);

      private:
        panels::Panels m_panel_type;
    };
} // namespace magnesia::activities::dataviewer
