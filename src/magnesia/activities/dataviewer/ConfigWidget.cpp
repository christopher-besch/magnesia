#include "ConfigWidget.hpp"

#include "../../Application.hpp"
#include "../../ConfigWidget.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"
#include "../../opcua_qt/abstraction/MessageSecurityMode.hpp"
#include "../../qt_version_check.hpp"
#include "DataViewer.hpp"

#include <utility>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QLoggingCategory>
#include <QMetaType>
#include <QPushButton>
#include <QSharedPointer>
#include <QSizePolicy>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtPreprocessorSupport>
#include <QtTypeTraits>
#else
#include <QtGlobal>
#endif

Q_LOGGING_CATEGORY(lcDVConfig, "magnesia.dataviewer.configwidget")

using magnesia::opcua_qt::Connection;
using magnesia::opcua_qt::ConnectionBuilder;
using magnesia::opcua_qt::Endpoint;

namespace magnesia::activities::dataviewer {
    namespace {
        QWidget* wrap_in_frame(QLayout* layout) {
            auto* frame = new QFrame;
            frame->setLayout(layout);
            frame->setFrameShape(QFrame::Shape::Box);
            return frame;
        }

        QWidget* wrap_in_frame(QWidget* widget) {
            auto* layout = new QVBoxLayout;
            layout->addWidget(widget);
            return wrap_in_frame(layout);
        }

        QString to_qstring(opcua_qt::MessageSecurityMode mode) {
            switch (mode) {
                case opcua_qt::MessageSecurityMode::INVALID:
                    return "Invalid";
                case opcua_qt::MessageSecurityMode::NONE:
                    return "None";
                case opcua_qt::MessageSecurityMode::SIGN:
                    return "Sign";
                case opcua_qt::MessageSecurityMode::SIGN_AND_ENCRYPT:
                    return "Sign & Encrypt";
            }
            Q_ASSERT(false);
            return {};
        }
    } // namespace

    ConfigWidget::ConfigWidget(QWidget* parent) : magnesia::ConfigWidget(parent) {
        auto* layout = new QHBoxLayout;

        auto* v_layout = new QVBoxLayout;

        v_layout->addWidget(buildQuickConnect());

        layout->addWidget(wrap_in_frame(v_layout), Qt::AlignCenter);
        layout->addWidget(wrap_in_frame(buildRecentConnections()), Qt::AlignCenter);

        setLayout(layout);
    }

    QWidget* ConfigWidget::buildQuickConnect() {
        auto* layout = new QFormLayout;
        {
            auto* label = new QLabel("<h2>Quick Connect</h2>");
            layout->addRow(label);
        }
        {
            m_address = new QLineEdit;
            layout->addRow("Address", m_address);
        }
        {
            m_certificate = new QComboBox;
            layout->addRow("Certificate", m_certificate);
        }
        {
            m_username = new QLineEdit;
            layout->addRow("Username", m_username);
        }
        {
            m_password = new QLineEdit;
            m_password->setEchoMode(QLineEdit::Password);
            layout->addRow("Password", m_password);
        }
        {
            auto* button = new QPushButton("Find Endpoints");
            connect(button, &QPushButton::clicked, this, &ConfigWidget::onFindEndpoints);
            layout->addRow(button);
        }
        {
            m_endpoint_selector = new QTableView;
            m_endpoint_selector->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_endpoint_selector->setSelectionMode(QAbstractItemView::SingleSelection);
            m_endpoint_selector->verticalHeader()->setHidden(true);
            m_endpoint_selector_model = new detail::EndpointTableModel(this);
            m_endpoint_selector->setModel(m_endpoint_selector_model);
            m_endpoint_selector->horizontalHeader()->setStretchLastSection(true);
            m_endpoint_selector->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            m_endpoint_selector->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

            connect(m_endpoint_selector->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                    [this] { m_connect_button->setEnabled(true); });
            layout->addRow(m_endpoint_selector);
        }
        {
            m_connect_button = new QPushButton("Connect");
            m_connect_button->setEnabled(false);
            connect(m_connect_button, &QPushButton::clicked, this, &ConfigWidget::onConnect);
            layout->addRow(m_connect_button);
        }

        auto* widget = new QWidget;
        widget->setLayout(layout);
        return widget;
    }

    QWidget* ConfigWidget::buildRecentConnections() {
        Q_UNUSED(this);
        return new QLabel("Recent Connections");
    }

    void ConfigWidget::reset() {
        m_current_connection_builder = nullptr;
        m_endpoint_selector_model->clear();
        m_connect_button->setEnabled(false);
    }

    void ConfigWidget::onFindEndpoints() {
        qCDebug(lcDVConfig) << "finding endpoints for:"                           //
                            << "\n  address:" << m_address->text()                //
                            << "\n  certificate:" << m_certificate->currentData() //
                            << "\n  username:" << m_username->text()              //
                            << "\n  password:" << m_password->text();

        auto builder = m_current_connection_builder = QSharedPointer<ConnectionBuilder>{new ConnectionBuilder};

        // TODO: certificate
        builder->url(m_address->text());
        if (auto username = m_username->text(); !username.isEmpty()) {
            builder->username(m_username->text());
            builder->password(m_password->text());
        }
        // FIXME: the connection builder is never destroyed, as there is a cyclical reference between signal connection
        // and lambda
        connect(builder.get(), &ConnectionBuilder::endpointsFound, this,
                [this, builder](const QList<Endpoint>& endpoints) {
                    if (m_current_connection_builder != builder) {
                        // TODO: cleanup?
                        return;
                    }
                    onEndpointsFound(endpoints);
                });
        builder->findEndpoints();

        m_connect_button->setEnabled(false);
    }

    void ConfigWidget::onEndpointsFound(const QList<Endpoint>& endpoints) {
        qCDebug(lcDVConfig) << "endpoints:";
        for (const auto& endpoint : endpoints) {
            qCDebug(lcDVConfig) << "  endpoint:" << endpoint.getEndpointUrl()
                                << "security:" << endpoint.getSecurityPolicyUri()
                                << "mode:" << to_qstring(endpoint.getSecurityMode());
        }

        m_endpoint_selector_model->setEndpoints(endpoints);
    }

    void ConfigWidget::onConnect() {
        auto index              = m_endpoint_selector->currentIndex();
        auto endpoint_selection = m_endpoint_selector->model()->data(index, Qt::UserRole);
        Q_ASSERT(endpoint_selection.userType() == QMetaType::fromType<Endpoint>().id());
        auto endpoint = *static_cast<Endpoint*>(endpoint_selection.data());
        qCDebug(lcDVConfig) << "connecting to:"                                   //
                            << "\n  address:" << m_address->text()                //
                            << "\n  certificate:" << m_certificate->currentData() //
                            << "\n  username:" << m_username->text()              //
                            << "\n  password:" << m_password->text()              //
                            << "\n  endpoint:" << index.row();

        m_current_connection_builder->endpoint(endpoint);
        auto* logger = new opcua_qt::Logger;
        m_current_connection_builder->logger(logger);
        // TODO: where and when should the connection be closed
        auto* connection = m_current_connection_builder->build();
        Q_ASSERT(connection != nullptr);
        connect(connection, &Connection::connected, this, [this, connection, logger]() {
            reset();
            Application::instance().openActivity(new DataViewer(connection, logger), "DataViewer");
        });
        m_connect_button->setEnabled(false);
        connection->connectAndRun();
    }

    namespace detail {
        int EndpointTableModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_endpoints.size());
        }

        int EndpointTableModel::columnCount(const QModelIndex& /*parent*/) const {
            return 3;
        }

        QVariant EndpointTableModel::data(const QModelIndex& index, int role) const {
            Q_ASSERT(checkIndex(index));

            const auto& endpoint = m_endpoints[index.row()];
            const auto  column   = index.column();

            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                    case 0:
                        return endpoint.getEndpointUrl();
                    case 1:
                        return endpoint.getSecurityPolicyUri().remove("http://opcfoundation.org/UA/SecurityPolicy#");
                    case 2:
                        return to_qstring(endpoint.getSecurityMode());
                    default:
                        Q_ASSERT(false);
                }
            } else if (role == Qt::ToolTipRole && column == 1) {
                return endpoint.getSecurityPolicyUri();
            } else if (role == Qt::UserRole) {
                return QVariant::fromValue(endpoint);
            }
            return {};
        }

        QVariant EndpointTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                switch (section) {
                    case 0:
                        return "Endpoint URL";
                    case 1:
                        return "Security Policy URI";
                    case 2:
                        return "Security Mode";
                    default:
                        Q_ASSERT(false);
                }
            }
            return {};
        }

        void EndpointTableModel::setEndpoints(QList<opcua_qt::Endpoint> endpoints) {
            beginResetModel();
            m_endpoints = std::move(endpoints);
            endResetModel();
        }

        void EndpointTableModel::clear() {
            setEndpoints({});
        }
    } // namespace detail
} // namespace magnesia::activities::dataviewer
