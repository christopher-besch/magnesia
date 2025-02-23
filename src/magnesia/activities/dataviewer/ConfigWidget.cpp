#include "ConfigWidget.hpp"

#include "../../Application.hpp"
#include "../../ConfigWidget.hpp"
#include "../../HistoricServerConnection.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/ConnectionBuilder.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../opcua_qt/abstraction/Endpoint.hpp"
#include "../../opcua_qt/abstraction/MessageSecurityMode.hpp"
#include "../../qt_version_check.hpp"
#include "DataViewer.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Result.h>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAbstractListModel>
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
#include <QLoggingCategory>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QSslCertificate>
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

namespace {
    Q_LOGGING_CATEGORY(lc_dv_config, "magnesia.dataviewer.configwidget")
} // namespace

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

        bool record_recent_connection(const ConnectionBuilder& builder, std::optional<StorageId> old = std::nullopt) {
            const auto& server_url = builder.getUrl();
            if (!server_url.has_value()) {
                return false;
            }

            const auto& endpoint = builder.getEndpoint();
            if (!endpoint.has_value()) {
                return false;
            }

            Application::instance().getStorageManager().storeHistoricServerConnection({
                .server_url                     = *server_url,
                .endpoint_url                   = endpoint->getEndpointUrl(),
                .endpoint_security_policy_uri   = endpoint->getSecurityPolicyUri(),
                .endpoint_message_security_mode = endpoint->getSecurityMode(),
                .username                       = builder.getUsername(),
                .password                       = builder.getPassword(),
                .application_certificate_id     = builder.getCertificate(),
                .trust_list_certificate_ids     = builder.getTrustList(),
                .revoked_list_certificate_ids   = builder.getRevokedList(),
                .last_layout_id                 = {}, // TODO
                .last_layout_group              = {}, // TODO
                .last_layout_domain             = {}, // TODO
                .last_used                      = QDateTime::currentDateTimeUtc(),
            });

            if (old.has_value()) {
                Application::instance().getStorageManager().deleteHistoricServerConnection(*old);
            }

            return true;
        }

        void open_dataviewer(Connection* connection, opcua_qt::Logger* logger) {
            Application::instance().openActivity(new DataViewer(connection, logger), "DataViewer",
                                                 connection->getEndpointUrl().toString());
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
            m_certificate       = new QComboBox;
            m_certificate_model = new detail::CertificateModel(this);
            m_certificate->setModel(m_certificate_model);
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
            m_endpoint_selector->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            m_endpoint_selector->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_endpoint_selector->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

            connect(m_endpoint_selector->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                    [this] { m_connect_button->setEnabled(true); });
            connect(m_endpoint_selector, &QTableView::activated, this, &ConfigWidget::onConnect);
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

        table->setContextMenuPolicy(Qt::ActionsContextMenu);
        auto* remove = table->addAction("Remove selected", Qt::Key_Delete, this, [table] {
            if (!table->selectionModel()->hasSelection()) {
                return;
            }
            auto row = table->selectionModel()->currentIndex().row();
            table->model()->removeRow(row);
        });
        remove->setShortcutContext(Qt::WidgetWithChildrenShortcut);

        connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                [this, model](const QModelIndex& current) {
                    if (!model->checkIndex(current, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
                        return;
                    }
                    qCDebug(lc_dv_config) << "Recent connection selection changed to" << current;
                    auto connection = model->data(current, detail::HistoricServerConnectionModel::ConnectionRole)
                                          .value<HistoricServerConnection>();

                    m_address->setText(connection.server_url.toString());
                    m_username->setText(connection.username.value_or(""));
                    m_password->setText(connection.password.value_or(""));
                    if (connection.application_certificate_id.has_value()) {
                        m_certificate->setCurrentIndex(
                            m_certificate_model->rowIndex(*connection.application_certificate_id));
                    }
                });

        connect(table, &QTableView::activated, this, [this, model](const QModelIndex& index) {
            qCDebug(lc_dv_config) << "Recent connection activated" << index;
            auto historic = model->data(index, detail::HistoricServerConnectionModel::ConnectionRole)
                                .value<HistoricServerConnection>();
            auto conid = model->data(index, detail::HistoricServerConnectionModel::ConnectionIdRole).value<StorageId>();

            auto builder = std::make_unique<ConnectionBuilder>();
            (*builder)
                .logger(new opcua_qt::Logger)
                .url(historic.server_url)
                .trustList(historic.trust_list_certificate_ids)
                .revokedList(historic.revoked_list_certificate_ids)
                .endpoint(Endpoint{
                    historic.endpoint_url,
                    historic.endpoint_security_policy_uri,
                    historic.endpoint_message_security_mode,
                });
            if (historic.application_certificate_id.has_value()) {
                builder->certificate(*historic.application_certificate_id);
            }
            if (historic.username.has_value()) {
                builder->username(*historic.username);
            }
            if (historic.password.has_value()) {
                builder->password(*historic.password);
            }

            auto* connection = builder->build();
            Q_ASSERT(connection != nullptr);
            // FIXME: the builder is only destroyed when the dataviewer is closed, as the lambda is destroyed when the
            // signal connection is destroyed
            connect(connection, &Connection::connected, this, [connection, conid, builder = std::move(builder)] {
                record_recent_connection(*builder, conid);
                open_dataviewer(connection, builder->getLogger());
            });
            connection->connectAndRun();
        });

        layout->addWidget(table);

        return layout;
    }

    void ConfigWidget::reset() {
        m_current_connection_builder.reset();
        m_endpoint_selector_model->clear();
        m_connect_button->setEnabled(false);
    }

    void ConfigWidget::onFindEndpoints() {
        qCDebug(lc_dv_config) << "finding endpoints for:"                           //
                              << "\n  address:" << m_address->text()                //
                              << "\n  certificate:" << m_certificate->currentData() //
                              << "\n  username:" << m_username->text()              //
                              << "\n  password:" << m_password->text();

        m_current_connection_builder = std::make_shared<ConnectionBuilder>();
        m_current_connection_builder->logger(new opcua_qt::Logger);

        m_current_connection_builder->url(m_address->text());
        if (auto username = m_username->text(); !username.isEmpty()) {
            m_current_connection_builder->username(m_username->text());
            m_current_connection_builder->password(m_password->text());
        }
        // FIXME: the connection builder is never destroyed, as there is a cyclical reference between signal connection
        // and lambda
        connect(m_current_connection_builder.get(), &ConnectionBuilder::endpointsFound, this,
                [this, builder = m_current_connection_builder](const opcua::Result<std::vector<Endpoint>>& endpoints) {
                    if (m_current_connection_builder != builder) {
                        // TODO: cleanup?
                        return;
                    }
                    onEndpointsFound(endpoints);
                });
        m_current_connection_builder->findEndpoints();

        m_connect_button->setEnabled(false);
    }

    void ConfigWidget::onEndpointsFound(const opcua::Result<std::vector<Endpoint>>& result) {
        if (result.hasValue()) {
            const auto& endpoints = result.value();
            qCDebug(lc_dv_config) << "endpoints:";
            for (const auto& endpoint : endpoints) {
                qCDebug(lc_dv_config) << "  endpoint:" << endpoint.getEndpointUrl()
                                      << "security:" << endpoint.getSecurityPolicyUri()
                                      << "mode:" << to_qstring(endpoint.getSecurityMode());
            }

            m_endpoint_selector_model->setEndpoints(endpoints);
        } else {
            qCWarning(lc_dv_config) << "Failed to find endpoints:" << QUtf8StringView{result.code().name()};
            // TODO: display warning message to the user
        }
    }

    void ConfigWidget::onConnect() {
        auto index              = m_endpoint_selector->currentIndex();
        auto endpoint_selection = m_endpoint_selector->model()->data(index, Qt::UserRole);
        Q_ASSERT(endpoint_selection.userType() == QMetaType::fromType<Endpoint>().id());
        auto endpoint = *static_cast<Endpoint*>(endpoint_selection.data());
        qCDebug(lc_dv_config) << "connecting to:"                                   //
                              << "\n  address:" << m_address->text()                //
                              << "\n  certificate:" << m_certificate->currentData() //
                              << "\n  username:" << m_username->text()              //
                              << "\n  password:" << m_password->text()              //
                              << "\n  endpoint:" << index.row();

        m_current_connection_builder->endpoint(endpoint);
        if (auto cert = m_certificate->currentData(); cert.isValid()) {
            m_current_connection_builder->certificate(cert.value<StorageId>());
        }

        auto* connection = m_current_connection_builder->build();
        Q_ASSERT(connection != nullptr);
        // FIXME: the builder can only destroyed when the dataviewer is closed, as the lambda is destroyed when the
        // signal connection is destroyed
        connect(connection, &Connection::connected, this, [this, connection, builder = m_current_connection_builder] {
            record_recent_connection(*builder);
            reset();
            open_dataviewer(connection, builder->getLogger());
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

            const auto& endpoint = m_endpoints[static_cast<std::size_t>(index.row())];
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

        void EndpointTableModel::setEndpoints(std::vector<opcua_qt::Endpoint> endpoints) {
            beginResetModel();
            m_endpoints = std::move(endpoints);
            endResetModel();
        }

        void EndpointTableModel::clear() {
            setEndpoints({});
        }

        CertificateModel::CertificateModel(QObject* parent)
            : QAbstractListModel(parent),
              m_certificates(Application::instance().getStorageManager().getAllApplicationCertificates()) {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::applicationCertificateChanged, this,
                    &CertificateModel::onApplicationCertificateChanged);
        }

        int CertificateModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_certificates.size());
        }

        QVariant CertificateModel::data(const QModelIndex& index, int role) const {
            if (!checkIndex(index)) {
                return {};
            }

            if (role == Qt::DisplayRole) {
                auto info = m_certificates[static_cast<std::size_t>(index.row())].second.getCertificate().subjectInfo(
                    QSslCertificate::CommonName);
                // TODO: what to do with the other info?
                return info.front();
            }

            if (role == Qt::UserRole) {
                return QVariant::fromValue(m_certificates[static_cast<std::size_t>(index.row())].first);
            }

            return {};
        }

        bool CertificateModel::removeRows(int row, int count, const QModelIndex& parent) {
            if (row < 0 || rowCount() <= row) {
                return false;
            }
            if (count != 1) {
                return false;
            }

            auto cert = m_certificates[static_cast<std::size_t>(row)];

            beginRemoveRows(parent, row, row + count - 1);
            m_certificates.erase(m_certificates.begin() + row);
            Application::instance().getStorageManager().deleteHistoricServerConnection(cert.first);
            endRemoveRows();

            return true;
        }

        void CertificateModel::addCertificate(StorageId cert_id) {
            auto cert = Application::instance().getStorageManager().getApplicationCertificate(cert_id);
            Q_ASSERT(cert.has_value());

            const int row = static_cast<int>(m_certificates.size());
            beginInsertRows({}, row, row);
            m_certificates.emplace_back(cert_id, *cert);
            endInsertRows();
        }

        int CertificateModel::rowIndex(StorageId cert_id) const {
            auto iter = std::ranges::find(m_certificates, cert_id, &decltype(m_certificates)::value_type::first);
            if (iter == m_certificates.end()) {
                return -1;
            }

            return static_cast<int>(std::distance(m_certificates.cbegin(), iter));
        }

        void CertificateModel::onApplicationCertificateChanged(StorageId cert_id, StorageChange type) {
            switch (type) {
                case StorageChange::Created:
                    addCertificate(cert_id);
                    break;

                case StorageChange::Deleted:
                    removeRow(rowIndex(cert_id));
                    break;

                case StorageChange::Modified:
                    // ApplicationCertificates aren't modified
                    Q_ASSERT(false);
                    break;
            }
        }

        HistoricServerConnectionModel::HistoricServerConnectionModel(QObject* parent)
            : QAbstractTableModel(parent),
              m_connections(Application::instance().getStorageManager().getAllHistoricServerConnections()) {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::historicServerConnectionChanged, this,
                    &HistoricServerConnectionModel::onHistoricServerConnectionChanged);

            std::ranges::sort(m_connections, std::ranges::greater{},
                              [](const auto& con) { return con.second.last_used; });
        }

        int HistoricServerConnectionModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_connections.size());
        }

        int HistoricServerConnectionModel::columnCount(const QModelIndex& /*parent*/) const {
            return COLUMN_COUNT;
        }

        QVariant HistoricServerConnectionModel::data(const QModelIndex& index, int role) const {
            if (!checkIndex(index)) {
                return {};
            }

            const auto& connection = m_connections[static_cast<std::size_t>(index.row())];

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
            } else if (role == Qt::ToolTipRole && index.column() == EndpointSecurityPolicyColumn) {
                return connection.second.endpoint_security_policy_uri;
            } else if (role == ConnectionRole) {
                return QVariant::fromValue(connection.second);
            } else if (role == ConnectionIdRole) {
                return QVariant::fromValue(connection.first);
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

        bool HistoricServerConnectionModel::removeRows(int row, int count, const QModelIndex& parent) {
            if (row < 0 || rowCount() <= row) {
                return false;
            }
            if (count != 1) {
                return false;
            }

            auto connection = m_connections[static_cast<std::size_t>(row)];

            beginRemoveRows(parent, row, row + count - 1);
            m_connections.erase(m_connections.begin() + row);
            Application::instance().getStorageManager().deleteHistoricServerConnection(connection.first);
            endRemoveRows();

            return true;
        }

        void HistoricServerConnectionModel::addConnection(StorageId connection_id) {
            auto connection = Application::instance().getStorageManager().getHistoricServerConnection(connection_id);
            Q_ASSERT(connection.has_value());

            auto iter =
                // NOLINTNEXTLINE(misc-include-cleaner): lower_bound is provided by <algorithm>
                std::ranges::lower_bound(m_connections, connection->last_used, std::ranges::greater{},
                                         [](const auto& con) { return con.second.last_used; });
            auto row = static_cast<int>(std::distance(m_connections.begin(), iter));

            beginInsertRows({}, row, row);
            m_connections.emplace(iter, connection_id, *std::move(connection));
            endInsertRows();
        }

        int HistoricServerConnectionModel::rowIndex(StorageId connection_id) const {
            auto iter = std::ranges::find(m_connections, connection_id, &decltype(m_connections)::value_type::first);
            if (iter == m_connections.end()) {
                return -1;
            }

            return static_cast<int>(std::distance(m_connections.cbegin(), iter));
        }

        void HistoricServerConnectionModel::onHistoricServerConnectionChanged(StorageId     connection_id,
                                                                              StorageChange type) {
            switch (type) {
                case StorageChange::Created:
                    addConnection(connection_id);
                    break;

                case StorageChange::Deleted:
                    removeRow(rowIndex(connection_id));
                    break;

                case StorageChange::Modified:
                    // HistoricServerConnections aren't modified
                    Q_ASSERT(false);
                    break;
            }
        }
    } // namespace detail
} // namespace magnesia::activities::dataviewer
