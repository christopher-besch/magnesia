#pragma once

#include "../../opcua_qt/abstraction/NodeId.hpp"
#include "PanelMetadata.hpp"
#include "dataviewer_fwd.hpp"

#include <QJsonObject>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    /**
     * @class Panel
     * @brief Base class for implementing panels for the `DataViewer`. Provides an abstraction for inter-panel
     * communication.
     */
    class Panel : public QWidget {
        Q_OBJECT

      public:
        /**
         * Saves the current panel state.
         *
         * @return JSON object containing the state of the panel.
         */
        [[nodiscard]] virtual QJsonObject saveState() const;

        /**
         * Restores the state of the panel from a former layout.
         *
         * @param data JSON string containing the state.
         * @return whether or not the restore succeeded.
         */
        [[nodiscard]] virtual bool restoreState(const QJsonObject& data);

        /**
         * Retrieves the metadata of the panel.
         */
        [[nodiscard]] const PanelMetadata& metadata() const noexcept;

      signals:
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients);

      protected:
        /**
         * @param dataviewer the `DataViewer` this panel belongs to.
         * @param panel the type of this panel. Requires an entry in the `panels::PanelType` enum.
         * @param metadata the metadata of this panel
         * @param parent the parent QWidget passed to Qt.
         */
        explicit Panel(DataViewer* dataviewer, panels::PanelType panel, PanelMetadata metadata,
                       QWidget* parent = nullptr);

        /**
         * Retrieves the Dataviewer displaying the panel.
         *
         * @return DataViewer displaying the panel.
         */
        [[nodiscard]] DataViewer* getDataViewer() const noexcept;

      protected slots:
        /**
         * Called when any Panel signals that a node was selected. The default implementation filters calls to the ones
         * targeted at this panels type and passes them to `Panel::selectNode`. You should not need to override this
         * function directly, see `Panel::selectNode` instead.
         *
         * @param node the selected node's id.
         * @param recipients the types of panels that are targeted by this call.
         */
        virtual void selectNodeAll(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients);
        /**
         * Called when any Panel signals that a node was selected and targets this panel's type. The default
         * implementation is a no-op.
         *
         * @param node the selected node's id.
         */
        virtual void selectNode(const opcua_qt::abstraction::NodeId& node);

      private:
        panels::PanelType m_panel_type;
        PanelMetadata     m_metadata;
        DataViewer*       m_dataviewer;
    };
} // namespace magnesia::activities::dataviewer
