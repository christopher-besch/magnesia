#pragma once

#include "../../Activity.hpp"
#include "../../Layout.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../opcua_qt/abstraction/NodeId.hpp"
#include "dataviewer_fwd.hpp"

#include <utility>
#include <vector>

#include <QAbstractListModel>
#include <QLayout>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    /**
     * @class DataViewer
     * @brief Main activity that allows to inspect and interact with OPC UA servers.
     */
    class DataViewer : public Activity {
        Q_OBJECT

        static constexpr auto s_storage_domain = "DataViewer";
        static constexpr auto s_layout_group   = "main";

      public:
        /**
         * @param connection Connection to an OPC UA server.
         * @param logger     Logger of the OPC UA connection.
         * @param parent     Parent of the activity.
         */
        explicit DataViewer(opcua_qt::Connection* connection, opcua_qt::Logger* logger, QWidget* parent = nullptr);

        /**
         * Retrieves the OPC UA server connection.
         *
         * @return The connection.
         */
        [[nodiscard]] opcua_qt::Connection* getConnection() const;

        /**
         * Retrieves the logger of the connection.
         *
         * @return The logger.
         */
        [[nodiscard]] opcua_qt::Logger* getLogger() const;

      signals:
        /**
         * Mediator signal to enable communication between panels.
         *
         * @param node the selected node
         * @param recipients the target panel types
         */
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::PanelTypes recipients);

      private:
        QLayout* buildLayoutSelector();

      private:
        layout::PanelLayout*  m_root_layout;
        opcua_qt::Connection* m_connection;
        opcua_qt::Logger*     m_logger;

        int m_old_layout_index{-1};
    };

    namespace detail {
        /**
         * @class LayoutSelectorModel
         * @brief Class for managing the current layout inside a DataViewer.
         */
        class LayoutSelectorModel : public QAbstractListModel {
            Q_OBJECT

          public:
            /**
             * @param domain Domain of the Layouts.
             * @param group Group containing the layout.
             * @param parent Parent of the widget.
             */
            explicit LayoutSelectorModel(Domain domain, LayoutGroup group, QObject* parent = nullptr);

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

            /**
             * Saves the layout to backing storage, adds it to the model and returns it's new index.
             *
             * @param layout the layout to add
             * @returns the new index of the layout.
             */
            int addLayout(const Layout& layout);

          public:
            enum {
                /// data role to get the actual layout data
                LayoutRole = Qt::UserRole,
                /// data role to check if a layout can be deleted
                DeletableRole,
            };

          private:
            void              addLayout(StorageId layout_id);
            [[nodiscard]] int rowIndex(StorageId layout_id) const;

          private slots:
            /**
             * Updates the Layout.
             * @param layout_id Id of the layout.
             * @param group Group of the layout.
             * @param domain Domain of the layout.
             * @param type Type of the change.
             */
            void onLayoutChanged(StorageId layout_id, const LayoutGroup& group, const Domain& domain,
                                 StorageChange type);

          private:
            Domain                                    m_domain;
            LayoutGroup                               m_group;
            std::vector<std::pair<StorageId, Layout>> m_layouts;
            std::vector<Layout>                       m_virtual_layouts;
        };
    } // namespace detail
} // namespace magnesia::activities::dataviewer
