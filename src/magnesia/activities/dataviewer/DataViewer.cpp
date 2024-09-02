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
#include <cstddef>
#include <iterator>
#include <utility>

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QMessageBox>
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
        auto* model           = new detail::LayoutSelectorModel(s_storage_domain, s_layout_group, layout_selector);
        layout->addWidget(layout_selector);

        auto* delete_button = new QPushButton("Remove Layout");
        layout->addWidget(delete_button);

        auto* save_edit = new QLineEdit;
        save_edit->hide();
        layout->addWidget(save_edit);

        auto* save_button = new QPushButton("Save");
        save_button->hide();
        layout->addWidget(save_button);

        auto* abort_button = new QPushButton("Abort");
        abort_button->hide();
        layout->addWidget(abort_button);

        connect(layout_selector, &QComboBox::currentIndexChanged, this,
                [this, layout_selector, model, delete_button, save_edit, save_button, abort_button](int index) {
                    if (index == layout_selector->model()->rowCount() - 1) {
                        layout_selector->hide();
                        delete_button->hide();
                        save_edit->show();
                        save_button->show();
                        abort_button->show();
                        save_edit->setFocus(Qt::FocusReason::OtherFocusReason);
                        return;
                    }

                    auto model_index = model->index(index, 0);

                    auto deletable = model->data(model_index, detail::LayoutSelectorModel::DeletableRole).toBool();
                    delete_button->setEnabled(deletable);

                    if (index == m_old_layout_index) {
                        return;
                    }

                    qCDebug(lc_data_viewer) << "selected layout" << index;
                    auto state = model->data(model_index, detail::LayoutSelectorModel::LayoutRole);
                    m_root_layout->restoreState(state.toJsonDocument());
                    m_old_layout_index = index;
                });

        connect(save_button, &QPushButton::clicked, this,
                [this, layout_selector, model, delete_button, save_edit, save_button, abort_button] {
                    auto state = m_root_layout->saveState();
                    auto name  = save_edit->text();

                    qCDebug(lc_data_viewer) << "saving layout" << name << "with json_data" << state;

                    auto index = model->addLayout({
                        .name      = name,
                        .json_data = state,
                    });

                    save_edit->clear();
                    layout_selector->show();
                    delete_button->show();
                    save_edit->hide();
                    save_button->hide();
                    abort_button->hide();
                    m_old_layout_index = index;
                    layout_selector->setFocus(Qt::FocusReason::OtherFocusReason);
                    layout_selector->setCurrentIndex(index);
                });

        connect(abort_button, &QPushButton::clicked, this,
                [this, layout_selector, delete_button, save_edit, save_button, abort_button] {
                    save_edit->clear();
                    layout_selector->show();
                    delete_button->show();
                    save_edit->hide();
                    save_button->hide();
                    abort_button->hide();
                    layout_selector->setFocus(Qt::FocusReason::OtherFocusReason);
                    layout_selector->setCurrentIndex(m_old_layout_index);
                });

        connect(delete_button, &QPushButton::clicked, this, [this, layout_selector] {
            auto text = QString("Do you really want to delete the currently selected layout '%1' ?")
                            .arg(layout_selector->currentText());
            if (QMessageBox::Yes != QMessageBox::question(this, "Delete current layout?", text)) {
                return;
            }

            auto current_index = layout_selector->currentIndex();
            // TODO: get rid of magic comparisons
            // check if the current item is one before "<Save Layout>" and prevent that being selected when the current
            // item is removed
            if (current_index == layout_selector->model()->rowCount() - 2) {
                layout_selector->setCurrentIndex(current_index - 1);
            }
            layout_selector->removeItem(current_index);
        });

        layout_selector->setModel(model);

        return layout;
    }

    namespace detail {
        LayoutSelectorModel::LayoutSelectorModel(Domain domain, LayoutGroup group, QObject* parent)
            : QAbstractListModel(parent), m_domain(std::move(domain)), m_group(std::move(group)),
              m_layouts(Application::instance().getStorageManager().getAllLayouts(m_group, m_domain)),
              m_virtual_layouts{
                  {
                   .name      = "Default",
                   .json_data = QJsonDocument::fromJson(R"json(
                       {"children":[
                         {"children":[{"children":[
                           {"data":{},"id":"treeview","type":"panel"},
                           {"data":{},"id":"nodeview","type":"panel"},
                           {"children":[
                               {"data":{},"id":"attributeview","type":"panel"},
                               {"data":{},"id":"referenceview","type":"panel"}
                             ],"splitter_state":"AAAA/wAAAAEAAAACAAAANgAAADYA/////wEAAAACAA==","type":"layout"}
                           ],"splitter_state":"AAAA/wAAAAEAAAADAAABAgAAASQAAAEkAP////8BAAAAAQA=","type":"layout"},
                           {"data":{},"id":"logview","type":"panel"}
                         ],"splitter_state":"AAAA/wAAAAEAAAACAAAA5AAAADYA/////wEAAAACAA==","type":"layout"}
                       ],"splitter_state":"AAAA/wAAAAEAAAABAAABJAD/////AQAAAAEA","type":"layout"}
                   )json"),
                   }
        } {
            auto* storage_manager = &Application::instance().getStorageManager();
            connect(storage_manager, &StorageManager::layoutChanged, this, &LayoutSelectorModel::onLayoutChanged);
        }

        int LayoutSelectorModel::rowCount(const QModelIndex& /*parent*/) const {
            return static_cast<int>(m_virtual_layouts.size() + m_layouts.size() + 1);
        }

        QVariant LayoutSelectorModel::data(const QModelIndex& index, int role) const {
            if (!checkIndex(index)) {
                return {};
            }

            auto row = static_cast<std::size_t>(index.row());

            if (role == Qt::DisplayRole) {
                if (row < m_virtual_layouts.size()) {
                    return m_virtual_layouts[row].name;
                }

                if (row < m_virtual_layouts.size() + m_layouts.size()) {
                    return m_layouts[row - m_virtual_layouts.size()].second.name;
                }

                if (index.row() == rowCount() - 1) {
                    return "<Save Layout>";
                }
            }

            if (role == LayoutRole) {
                if (row < m_virtual_layouts.size()) {
                    return m_virtual_layouts[row].json_data;
                }

                if (row < m_virtual_layouts.size() + m_layouts.size()) {
                    return m_layouts[row - m_virtual_layouts.size()].second.json_data;
                }
            }

            if (role == DeletableRole) {
                return m_virtual_layouts.size() <= row && row < m_virtual_layouts.size() + m_layouts.size();
            }

            return {};
        }

        bool LayoutSelectorModel::removeRows(int row, int count, const QModelIndex& parent) {
            if (count != 1) {
                return false;
            }
            if (!data(index(row, 0), DeletableRole).toBool()) {
                return false;
            }

            auto layouts_index = static_cast<std::size_t>(row) - m_virtual_layouts.size();

            auto layout_id = m_layouts[layouts_index].first;

            beginRemoveRows(parent, row, row);
            m_layouts.erase(m_layouts.begin() + static_cast<std::ptrdiff_t>(layouts_index));
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

            const int row = static_cast<int>(m_layouts.size());
            beginInsertRows({}, row, row);
            m_layouts.emplace_back(layout_id, *layout);
            endInsertRows();
        }

        int LayoutSelectorModel::rowIndex(StorageId layout_id) const {
            auto iter = std::ranges::find(m_layouts, layout_id, &decltype(m_layouts)::value_type::first);
            if (iter == m_layouts.end()) {
                return -1;
            }

            return static_cast<int>(m_virtual_layouts.size()
                                    + static_cast<std::size_t>(std::distance(m_layouts.begin(), iter)));
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
