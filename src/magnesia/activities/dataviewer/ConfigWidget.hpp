#pragma once

#include "../../ConfigWidget.hpp"
#include "../../HistoricServerConnection.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/ApplicationCertificate.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <open62541pp/Result.h>

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QLayout>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

namespace magnesia::activities::dataviewer {
    namespace detail {
        class EndpointTableModel;
        class CertificateModel;
    } // namespace detail

    /**
     * @class ConfigWidget
     * @brief `magnesia::ConfigWidget` for the `DataViewer` activity.
     */
    class ConfigWidget : public magnesia::ConfigWidget {
        Q_OBJECT

      public:
        /**
         * @param parent Parent widget for the widget.
         */
        explicit ConfigWidget(QWidget* parent = nullptr);

      private:
        QLayout* buildQuickConnect();
        QLayout* buildRecentConnections();
        void     reset();

      private slots:
        void onFindEndpoints();
        void onEndpointsFound(const opcua::Result<std::vector<opcua_qt::Endpoint>>& result);
        void onConnect();

      private:
        std::size_t m_count{};

        std::shared_ptr<opcua_qt::ConnectionBuilder> m_current_connection_builder;

        // quick connect
        QLineEdit*                m_address{nullptr};
        detail::CertificateModel* m_certificate_model{nullptr};
        QComboBox*                m_certificate{nullptr};
        QLineEdit*                m_username{nullptr};
        QLineEdit*                m_password{nullptr};

        detail::EndpointTableModel* m_endpoint_selector_model{nullptr};
        QTableView*                 m_endpoint_selector{nullptr};

        QPushButton* m_connect_button{nullptr};
    };

    namespace detail {

        /**
         * @class EndpointTableModel
         * @brief Model for the EndpointTable in the ConfigWidget
         */
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
            /**
             * Sets the Endpoints of the endpoints of the table.
             *
             * @param endpoints List of endpoints.
             */
            void setEndpoints(std::vector<opcua_qt::Endpoint> endpoints);

            /**
             * Clears the table.
             */
            void clear();

          private:
            std::vector<opcua_qt::Endpoint> m_endpoints;
        };

        class CertificateModel : public QAbstractListModel {
            Q_OBJECT

          public:
            explicit CertificateModel(QObject* parent = nullptr);

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

            [[nodiscard]] int rowIndex(StorageId cert_id) const;

          private:
            void addCertificate(StorageId cert_id);

          private slots:
            void onApplicationCertificateChanged(StorageId cert_id, StorageChange type);

          private:
            std::vector<std::pair<StorageId, opcua_qt::ApplicationCertificate>> m_certificates;
        };

        /**
         * @class HistoricServerConnectionModel
         * @brief Model for a HistoricServerConnection in the ConfigWidget.
         */
        class HistoricServerConnectionModel : public QAbstractTableModel {
            Q_OBJECT

          public:
            /**
             * @param parent Parent of the Model.
             */
            explicit HistoricServerConnectionModel(QObject* parent = nullptr);

            [[nodiscard]] int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
            [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                              int role = Qt::DisplayRole) const override;

            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

          public:
            enum {
                /// data role for the HistoricServerConnection object associated with the index
                ConnectionRole = Qt::UserRole,
                /// data role for the HistoricServerConnection's StorageId associated with the index
                ConnectionIdRole,
            };

          private:
            void              addConnection(StorageId connection_id);
            [[nodiscard]] int rowIndex(StorageId connection_id) const;

          private slots:

            void onHistoricServerConnectionChanged(StorageId connection_id, StorageChange type);

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
            std::vector<std::pair<StorageId, HistoricServerConnection>> m_connections;
        };
    } // namespace detail
} // namespace magnesia::activities::dataviewer
