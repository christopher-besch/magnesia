#include "DataViewer.hpp"

#include "Activity.hpp"
#include "layout.hpp"
#include "opcua_qt/Connection.hpp"
#include "opcua_qt/Logger.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace magnesia::activities::dataviewer {
    DataViewer::DataViewer(opcua_qt::Connection* connection, opcua_qt::Logger* logger, QWidget* parent)
        : Activity(parent), m_connection(connection), m_logger(logger) {
        auto* layout = new QVBoxLayout;

        auto* address_layout = new QVBoxLayout;
        auto* address_view   = new QLineEdit;
        address_view->setReadOnly(true);
        address_view->setText(m_connection->getEndpointUrl().toString());
        address_layout->addWidget(address_view);
        layout->addLayout(address_layout);

        auto* panel_layout = new layout::PanelLayout(this, Qt::Horizontal, nullptr);

        layout->addWidget(panel_layout);
        panel_layout->addWidget();

        setLayout(layout);
    }

    opcua_qt::Connection* DataViewer::getConnection() const {
        return m_connection;
    }

    opcua_qt::Logger* DataViewer::getLogger() const {
        return m_logger;
    }
} // namespace magnesia::activities::dataviewer
