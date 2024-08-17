#pragma once

#include "../../Activity.hpp"
#include "../../Layout.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../opcua_qt/abstraction/NodeId.hpp"
#include "dataviewer_fwd.hpp"

#include <utility>

#include <QAbstractListModel>
#include <QLayout>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    /**
     * Main activity that allows to inspect and interact with OPC UA servers.
     */
    class DataViewer : public Activity {
        Q_OBJECT

        static constexpr auto s_storage_domain = "DataViewer";
        static constexpr auto s_layout_group   = "main";

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
        QLayout* buildLayoutSelector();

      private:
        layout::PanelLayout*  m_root_layout;
        opcua_qt::Connection* m_connection;
        opcua_qt::Logger*     m_logger;
    };

    namespace detail {
        class LayoutSelectorModel : public QAbstractListModel {
            Q_OBJECT

          public:
            explicit LayoutSelectorModel(Domain domain, LayoutGroup group, QObject* parent = nullptr);

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

          private slots:
            void reload();

          private:
            Domain                              m_domain;
            LayoutGroup                         m_group;
            QList<std::pair<StorageId, Layout>> m_layouts;
        };
    } // namespace detail
} // namespace magnesia::activities::dataviewer
