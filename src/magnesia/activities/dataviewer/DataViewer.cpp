#include "DataViewer.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "layout.hpp"

#include <utility>

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>

Q_LOGGING_CATEGORY(lcDataViewer, "magnesia.dataviewer.configwidget")

namespace magnesia::activities::dataviewer {
    namespace detail {
        class LayoutSelectorModel;
    } // namespace detail

    DataViewer::DataViewer(opcua_qt::Connection* connection, opcua_qt::Logger* logger, QWidget* parent)
        : Activity(parent), m_root_layout(new layout::PanelLayout(this, Qt::Horizontal, nullptr)),
          m_connection(connection), m_logger(logger) {
        auto* layout = new QVBoxLayout;
        // layout->setContentsMargins(2, 2, 2, 2);
        layout->setContentsMargins(4, 4, 4, 4);

        auto* address_layout = new QHBoxLayout;

        auto* address_view = new QLineEdit;
        address_view->setReadOnly(true);
        address_view->setText(m_connection->getEndpointUrl().toString());
        address_layout->addWidget(address_view, Qt::AlignCenter);

        address_layout->addLayout(buildLayoutSelector());

        layout->addLayout(address_layout);

        layout->addWidget(m_root_layout, Qt::AlignCenter);
        m_root_layout->addWidget();

        setLayout(layout);
    }

    opcua_qt::Connection* DataViewer::getConnection() const {
        return m_connection;
    }

    opcua_qt::Logger* DataViewer::getLogger() const {
        return m_logger;
    }

    QLayout* DataViewer::buildLayoutSelector() {
        auto* layout = new QHBoxLayout;

        auto* layout_selector = new QComboBox;
        layout_selector->setPlaceholderText("Select Layout");
        auto* model = new detail::LayoutSelectorModel(s_storage_domain, s_layout_group, layout_selector);
        layout_selector->setModel(model);
        layout_selector->setCurrentIndex(-1);
        layout->addWidget(layout_selector);

        auto* save_edit = new QLineEdit;
        save_edit->hide();
        layout->addWidget(save_edit);

        auto* save_button = new QPushButton("Save");
        save_button->hide();
        layout->addWidget(save_button);

        connect(layout_selector, &QComboBox::currentIndexChanged, this,
                [this, layout_selector, save_edit, save_button](int index) {
                    if (index == layout_selector->model()->rowCount() - 1) {
                        layout_selector->hide();
                        save_edit->show();
                        save_button->show();
                        save_edit->setFocus(Qt::FocusReason::OtherFocusReason);
                        return;
                    }

                    qCDebug(lcDataViewer) << "selected layout" << index;
                    auto model_index = layout_selector->model()->index(index, 0);
                    auto state       = layout_selector->model()->data(model_index, Qt::UserRole);
                    m_root_layout->restoreState(state.toJsonDocument());
                });

        connect(save_button, &QPushButton::clicked, this, [this, layout_selector, save_edit, save_button] {
            auto state = m_root_layout->saveState();
            auto name  = save_edit->text();

            qCDebug(lcDataViewer) << "saving layout" << name << "with json_data" << state;

            Application::instance().getStorageManager().storeLayout(
                {
                    .name      = name,
                    .json_data = state,
                },
                s_layout_group, s_storage_domain);

            save_edit->clear();
            layout_selector->show();
            save_edit->hide();
            save_button->hide();
            layout_selector->setFocus(Qt::FocusReason::OtherFocusReason);
        });

        return layout;
    }

    namespace detail {
        LayoutSelectorModel::LayoutSelectorModel(Domain domain, LayoutGroup group, QObject* parent)
            : QAbstractListModel(parent), m_domain(std::move(domain)), m_group(std::move(group)) {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::layoutChanged, this, &LayoutSelectorModel::reload);

            reload();
        }

        int LayoutSelectorModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_layouts.count()) + 1;
        }

        QVariant LayoutSelectorModel::data(const QModelIndex& index, int role) const {
            if (!checkIndex(index)) {
                return {};
            }

            if (role == Qt::DisplayRole) {
                if (index.row() == m_layouts.count()) {
                    return "<Save Layout>";
                }

                return m_layouts[index.row()].name;
            }
            if (role == Qt::UserRole) {
                if (index.row() < m_layouts.count()) {
                    return m_layouts[index.row()].json_data;
                }
            }

            return {};
        }

        void LayoutSelectorModel::reload() {
            // TODO: make sure the currently selected layout stays the same
            auto layouts = Application::instance().getStorageManager().getAllLayouts(m_group, m_domain);

            beginResetModel();
            m_layouts = std::move(layouts);
            endResetModel();
        }
    } // namespace detail
} // namespace magnesia::activities::dataviewer
