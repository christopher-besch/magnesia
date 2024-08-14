#pragma once

#include "../..//database_types.hpp"
#include "../../ConfigWidget.hpp"
#include "../../HistoricServerConnection.hpp"
#include "../../StorageManager.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"

#include <cstddef>
#include <utility>

#include <open62541pp/Result.h>

#include <QAbstractTableModel>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QModelIndex>
#include <QPushButton>
#include <QSharedPointer>
#include <QTableView>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    namespace detail {
        class EndpointTableModel;
    } // namespace detail

    /**
     * `magnesia::ConfigWidget` for the `DataViewer` activity.
     */
    class ConfigWidget : public magnesia::ConfigWidget {
        Q_OBJECT

      public:
        explicit ConfigWidget(QWidget* parent = nullptr);

      private:
        QLayout* buildQuickConnect();
        QLayout* buildRecentConnections();
        void     reset();
        bool     recordRecentConnection();

      private slots:
        void onFindEndpoints();
        void onEndpointsFound(const opcua::Result<QList<opcua_qt::Endpoint>>& result);
        void onConnect();

      private:
        std::size_t m_count{};

        QSharedPointer<opcua_qt::ConnectionBuilder> m_current_connection_builder;

        // quick connect
        QLineEdit* m_address{nullptr};
        QComboBox* m_certificate{nullptr};
        QLineEdit* m_username{nullptr};
        QLineEdit* m_password{nullptr};

        detail::EndpointTableModel* m_endpoint_selector_model{nullptr};
        QTableView*                 m_endpoint_selector{nullptr};

        QPushButton* m_connect_button{nullptr};
    };

    namespace detail {
        class EndpointTableModel : public QAbstractTableModel {
            Q_OBJECT

          public:
            using QAbstractTableModel::QAbstractTableModel;

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                              int role = Qt::DisplayRole) const override;

          public:
            void setEndpoints(QList<opcua_qt::Endpoint> endpoints);
            void clear();

          private:
            QList<opcua_qt::Endpoint> m_endpoints;
        };

        class HistoricServerConnectionModel : public QAbstractTableModel {
            Q_OBJECT

          public:
            explicit HistoricServerConnectionModel(QObject* parent = nullptr);

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                              int role = Qt::DisplayRole) const override;

            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

          private slots:
            void onHistoricServerConnectionChanged(StorageId historic_server_connection_id, StorageChange type);
            void reload();

          private:
            enum {
                EndpointUrlColumn,
                EndpointSecurityPolicyColumn,
                EndpointSecurityModeColumn,
                UsernameColumn,
                LastUsedColumn,

                COLUMN_COUNT,
            };

          private:
            QList<std::pair<StorageId, HistoricServerConnection>> m_connections;
        };
    } // namespace detail
} // namespace magnesia::activities::dataviewer
