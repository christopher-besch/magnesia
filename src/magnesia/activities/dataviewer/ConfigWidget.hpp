#pragma once

#include "../../ConfigWidget.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"

#include <cstddef>

#include <QAbstractItemModel>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
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
        QWidget* buildRecentConnections();
        void     reset();

      private slots:
        void onFindEndpoints();
        void onEndpointsFound(const QList<opcua_qt::Endpoint>& endpoints);
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
    } // namespace detail
} // namespace magnesia::activities::dataviewer
