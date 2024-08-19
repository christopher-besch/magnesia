#include "DataViewer.hpp"

#include "../../Activity.hpp"
#include "../../Application.hpp"
#include "../../Layout.hpp"
#include "../../StorageManager.hpp"
#include "../../database_types.hpp"
#include "../../opcua_qt/Connection.hpp"
#include "../../opcua_qt/Logger.hpp"
#include "../../qt_version_check.hpp"
#include "layout.hpp"

#include <algorithm>
#include <iterator>
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

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_data_viewer, "magnesia.dataviewer.configwidget")
} // namespace

namespace magnesia::activities::dataviewer {
    namespace detail {
        class LayoutSelectorModel;
    } // namespace detail

    DataViewer::DataViewer(opcua_qt::Connection* connection, opcua_qt::Logger* logger, QWidget* parent)
        : Activity(parent), m_root_layout(new layout::PanelLayout(this, Qt::Horizontal, nullptr)),
          m_connection(connection), m_logger(logger) {
        m_connection->setParent(this);
        m_logger->setParent(this);

        auto* layout = new QVBoxLayout;
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

                    qCDebug(lc_data_viewer) << "selected layout" << index;
                    auto model_index = layout_selector->model()->index(index, 0);
                    auto state       = layout_selector->model()->data(model_index, Qt::UserRole);
                    m_root_layout->restoreState(state.toJsonDocument());
                });

        connect(save_button, &QPushButton::clicked, this, [this, layout_selector, model, save_edit, save_button] {
            auto state = m_root_layout->saveState();
            auto name  = save_edit->text();

            qCDebug(lc_data_viewer) << "saving layout" << name << "with json_data" << state;

            auto index = model->addLayout({
                .name      = name,
                .json_data = state,
            });

            save_edit->clear();
            layout_selector->show();
            save_edit->hide();
            save_button->hide();
            layout_selector->setFocus(Qt::FocusReason::OtherFocusReason);
            layout_selector->setCurrentIndex(index);
        });

        return layout;
    }

    namespace detail {
        LayoutSelectorModel::LayoutSelectorModel(Domain domain, LayoutGroup group, QObject* parent)
            : QAbstractListModel(parent), m_domain(std::move(domain)), m_group(std::move(group)),
              m_layouts(Application::instance().getStorageManager().getAllLayouts(m_group, m_domain)) {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::layoutChanged, this, &LayoutSelectorModel::onLayoutChanged);
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

                return m_layouts[index.row()].second.name;
            }
            if (role == Qt::UserRole) {
                if (index.row() < m_layouts.count()) {
                    return m_layouts[index.row()].second.json_data;
                }
            }

            return {};
        }

        bool LayoutSelectorModel::removeRows(int row, int count, const QModelIndex& parent) {
            if (row < 0 || rowCount() <= row) {
                return false;
            }
            if (count != 1) {
                return false;
            }
            if (row == m_layouts.count()) {
                return false;
            }

            auto layout_id = m_layouts.at(row).first;

            beginRemoveRows(parent, row, row);
            m_layouts.remove(row);
            Application::instance().getStorageManager().deleteLayout(layout_id, m_group, m_domain);
            endRemoveRows();

            return true;
        }

        int LayoutSelectorModel::addLayout(const Layout& layout) {
            auto layout_id = Application::instance().getStorageManager().storeLayout(layout, m_group, m_domain);
            return rowIndex(layout_id);
        }

        void LayoutSelectorModel::addLayout(StorageId layout_id) {
            auto layout = Application::instance().getStorageManager().getLayout(layout_id, m_group, m_domain);
            Q_ASSERT(layout.has_value());

            const int row = static_cast<int>(m_layouts.count());
            beginInsertRows({}, row, row);
            m_layouts.emplaceBack(layout_id, *layout);
            endInsertRows();
        }

        int LayoutSelectorModel::rowIndex(StorageId layout_id) const {
            auto iter = std::ranges::find(std::as_const(m_layouts), layout_id, &decltype(m_layouts)::value_type::first);
            if (iter == m_layouts.cend()) {
                return -1;
            }

            return static_cast<int>(std::distance(m_layouts.cbegin(), iter));
        }

        void LayoutSelectorModel::onLayoutChanged(StorageId layout_id, const LayoutGroup& group, const Domain& domain,
                                                  StorageChange type) {
            if (group != m_group || domain != m_domain) {
                return;
            }

            switch (type) {
                case StorageChange::Created: {
                    addLayout(layout_id);
                    break;
                }
                case StorageChange::Deleted:
                    removeRow(rowIndex(layout_id));
                    break;

                case StorageChange::Modified:
                    // Layouts aren't modified
                    Q_ASSERT(false);
                    break;
            }
        }
    } // namespace detail
} // namespace magnesia::activities::dataviewer
