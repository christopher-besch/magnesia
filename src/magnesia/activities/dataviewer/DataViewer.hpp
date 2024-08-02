#pragma once

#include "Activity.hpp"
#include "activities/dataviewer/panels.hpp"
#include "opcua_qt/Connection.hpp"
#include "opcua_qt/Logger.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"

#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    /**
     * Main activity that allows to inspect and interact with OPC UA servers.
     */
    class DataViewer : public Activity {
        Q_OBJECT

      public:
        explicit DataViewer(opcua_qt::Connection* connection, opcua_qt::Logger* logger, QWidget* parent = nullptr);

        [[nodiscard]] opcua_qt::Connection* getConnection() const;
        [[nodiscard]] opcua_qt::Logger*     getLogger() const;

      signals:
        /**
         * Mediator signal to enable communication between panels.
         *
         * @param node the selected node
         * @param recipients the target panel types
         */
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::Panels recipients);

      private:
        opcua_qt::Connection* m_connection{nullptr};
        opcua_qt::Logger*     m_logger{nullptr};
    };
} // namespace magnesia::activities::dataviewer
