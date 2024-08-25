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
        void nodeSelected(const opcua_qt::abstraction::NodeId& node, panels::PanelType recipients);

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

            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

            /**
             * Saves the layout to backing storage, adds it to the model and returns it's new index.
             *
             * @param layout the layout to add
             * @returns the new index of the layout.
             */
            int addLayout(const Layout& layout);

          private:
            void              addLayout(StorageId layout_id);
            [[nodiscard]] int rowIndex(StorageId layout_id) const;

          private slots:
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
