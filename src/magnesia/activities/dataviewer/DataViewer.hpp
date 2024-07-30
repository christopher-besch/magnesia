#pragma once

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "ConfigWidget.hpp"
#include "activities/dataviewer/panels.hpp"
#include "opcua_qt/abstraction/NodeId.hpp"
#include "qt_version_check.hpp"

#include <cstddef>

#include <QWidget>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer {
    /**
     * Main activity that allows to inspect and interact with OPC UA servers.
     */
    class DataViewer : public Activity {
        Q_OBJECT

      public:
        explicit DataViewer(QWidget* parent = nullptr);

      signals:
        /**
         * Mediator signal to enable communication between panels.
         *
         * @param node the selected node
         * @param recipients the target panel types
         */
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::Panels recipients);
    };

    /**
     * `magnesia::ConfigWidget` for the `DataViewer` activity.
     */
    class ConfigWidget : public magnesia::ConfigWidget {
        Q_OBJECT

      public:
        explicit ConfigWidget(QWidget* parent = nullptr);

      private slots:
        void create();

      private:
        std::size_t m_count{};
    };

    inline constexpr ActivityMetadata metadata{
        .name                 = u"DataViewer",
        .global_init          = []() { qDebug() << "Initialized DataViewer"; },
        .create_config_widget = []() -> magnesia::ConfigWidget* { return new ConfigWidget; },
    };
} // namespace magnesia::activities::dataviewer
