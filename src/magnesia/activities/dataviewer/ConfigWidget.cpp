#include "ConfigWidget.hpp"

#include "../../Application.hpp"
#include "../../ConfigWidget.hpp"
#include "../../StorageManager.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"
#include "../../opcua_qt/abstraction/MessageSecurityMode.hpp"
#include "../../qt_version_check.hpp"
#include "DataViewer.hpp"

#include <algorithm>
#include <functional>
#include <utility>

#include <open62541pp/Result.h>

#include <QAbstractItemView>
#include <QAbstractTableModel>
#include <QByteArrayView>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QLoggingCategory>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QSharedPointer>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
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
            frame->setFrameShape(QFrame::Shape::StyledPanel);
            return frame;
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

        QString format_security_policy(QString uri) {
            return uri.remove("http://opcfoundation.org/UA/SecurityPolicy#");
        }
    } // namespace

    ConfigWidget::ConfigWidget(QWidget* parent) : magnesia::ConfigWidget(parent) {
        auto* layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);

        layout->addWidget(wrap_in_frame(buildQuickConnect()), Qt::AlignCenter);
        layout->addWidget(wrap_in_frame(buildRecentConnections()), Qt::AlignCenter);

        setLayout(layout);
    }

    QLayout* ConfigWidget::buildQuickConnect() {
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

        return layout;
    }

    QLayout* ConfigWidget::buildRecentConnections() {
        auto* layout = new QVBoxLayout;

        layout->addWidget(new QLabel("<h2>Recent Connections</h2>"));

        auto* table = new QTableView;

        table->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->verticalHeader()->setHidden(true);

        auto* model = new detail::HistoricServerConnectionModel(this);
        table->setModel(model);

        layout->addWidget(table);

        return layout;
    }

    void ConfigWidget::reset() {
        m_current_connection_builder = nullptr;
        m_endpoint_selector_model->clear();
        m_connect_button->setEnabled(false);
    }

    bool ConfigWidget::recordRecentConnection() {
        const auto& server_url = m_current_connection_builder->getUrl();
        if (!server_url.has_value()) {
            return false;
        }

        const auto& endpoint = m_current_connection_builder->getEndpoint();
        if (!endpoint.has_value()) {
            return false;
        }

        Application::instance().getStorageManager().storeHistoricServerConnection({
            .server_url                     = *server_url,
            .endpoint_url                   = endpoint->getEndpointUrl(),
            .endpoint_security_policy_uri   = endpoint->getSecurityPolicyUri(),
            .endpoint_message_security_mode = endpoint->getSecurityMode(),
            .username                       = m_current_connection_builder->getUsername(),
            .password                       = m_current_connection_builder->getPassword(),
            .application_certificate_id = {}, // TODO: m_current_connection_builder->getCertificate()->getCertificate()
            .trust_list_certificate_ids = {}, // TODO: m_current_connection_builder->getTrustList()
            .revoked_list_certificate_ids = {}, // TODO: m_current_connection_builder->getRevokedList()
            .last_layout_id               = {}, // TODO
            .last_layout_group            = {}, // TODO
            .last_layout_domain           = {}, // TODO
            .last_used                    = QDateTime::currentDateTimeUtc(),
        });

        return true;
    }

    void ConfigWidget::onFindEndpoints() {
        qCDebug(lcDVConfig) << "finding endpoints for:"                           //
                            << "\n  address:" << m_address->text()                //
                            << "\n  certificate:" << m_certificate->currentData() //
                            << "\n  username:" << m_username->text()              //
                            << "\n  password:" << m_password->text();

        auto builder = m_current_connection_builder = QSharedPointer<ConnectionBuilder>{new ConnectionBuilder};
        m_current_connection_builder->logger(new opcua_qt::Logger);

        // TODO: certificate
        builder->url(m_address->text());
        if (auto username = m_username->text(); !username.isEmpty()) {
            builder->username(m_username->text());
            builder->password(m_password->text());
        }
        // FIXME: the connection builder is never destroyed, as there is a cyclical reference between signal connection
        // and lambda
        connect(builder.get(), &ConnectionBuilder::endpointsFound, this,
                [this, builder](const opcua::Result<QList<Endpoint>>& endpoints) {
                    if (m_current_connection_builder != builder) {
                        // TODO: cleanup?
                        return;
                    }
                    onEndpointsFound(endpoints);
                });
        builder->findEndpoints();

        m_connect_button->setEnabled(false);
    }

    void ConfigWidget::onEndpointsFound(const opcua::Result<QList<Endpoint>>& result) {
        if (result.hasValue()) {
            const auto& endpoints = result.value();
            qCDebug(lcDVConfig) << "endpoints:";
            for (const auto& endpoint : endpoints) {
                qCDebug(lcDVConfig) << "  endpoint:" << endpoint.getEndpointUrl()
                                    << "security:" << endpoint.getSecurityPolicyUri()
                                    << "mode:" << to_qstring(endpoint.getSecurityMode());
            }

            m_endpoint_selector_model->setEndpoints(endpoints);
        } else {
            qCWarning(lcDVConfig) << "Failed to find endpoints:" << QUtf8StringView{result.code().name()};
            // TODO: display warning message to the user
        }
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

        auto* connection = m_current_connection_builder->build();
        Q_ASSERT(connection != nullptr);
        connect(connection, &Connection::connected, this,
                [this, connection, logger = m_current_connection_builder->getLogger()] {
                    recordRecentConnection();
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
                        return format_security_policy(endpoint.getSecurityPolicyUri());
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

        HistoricServerConnectionModel::HistoricServerConnectionModel(QObject* parent) : QAbstractTableModel(parent) {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::historicServerConnectionChanged, this,
                    &HistoricServerConnectionModel::reload);

            reload();
        }

        int HistoricServerConnectionModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_connections.count());
        }

        int HistoricServerConnectionModel::columnCount(const QModelIndex& /*parent*/) const {
            return COLUMN_COUNT;
        }

        QVariant HistoricServerConnectionModel::data(const QModelIndex& index, int role) const {
            if (!checkIndex(index)) {
                return {};
            }

            const auto& connection = m_connections[index.row()];

            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                    case EndpointUrlColumn:
                        return connection.second.endpoint_url;
                    case EndpointSecurityPolicyColumn:
                        return format_security_policy(connection.second.endpoint_security_policy_uri);
                    case EndpointSecurityModeColumn:
                        return to_qstring(connection.second.endpoint_message_security_mode);
                    case UsernameColumn:
                        return connection.second.username.value_or("");
                    case LastUsedColumn:
                        return connection.second.last_used.toLocalTime();
                    default:
                        Q_ASSERT(false);
                }
            }

            if (role == Qt::UserRole) {
                return QVariant::fromValue(connection.second);
            }

            return {};
        }

        QVariant HistoricServerConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
            if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
                return {};
            }

            switch (section) {
                case EndpointUrlColumn:
                    return "Endpoint URL";
                case EndpointSecurityPolicyColumn:
                    return "Security Policy URI";
                case EndpointSecurityModeColumn:
                    return "Security Mode";
                case UsernameColumn:
                    return "Username";
                case LastUsedColumn:
                    return "Last Used";
                default:
                    Q_ASSERT(false);
            }

            return {};
        }

        void HistoricServerConnectionModel::reload() {
            auto connections = Application::instance().getStorageManager().getAllHistoricServerConnections();
            std::ranges::sort(connections, std::ranges::greater{},
                              [](const auto& con) { return con.second.last_used; });

            beginResetModel();
            m_connections = std::move(connections);
            endResetModel();
        }
    } // namespace detail
} // namespace magnesia::activities::dataviewer
