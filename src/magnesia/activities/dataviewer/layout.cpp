#include "layout.hpp"

#include "../../qt_version_check.hpp"
#include "DataViewer.hpp"
#include "Panel.hpp"
#include "PanelMetadata.hpp"
#include "panels.hpp"

#include <functional>
#include <utility>

#include <QAction>
#include <QByteArray>
#include <QChildEvent>
#include <QComboBox>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLoggingCategory>
#include <QObject>
#include <QSplitter>
#include <QString>
#include <QStringView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_layout, "magnesia.dataviewer.layout")
} // namespace

namespace magnesia::activities::dataviewer::layout {
    PanelWrapper::PanelWrapper(DataViewer* dataviewer, class Panel* panel, QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout), m_widget(panel), m_dataviewer(dataviewer) {
        Q_ASSERT(dataviewer != nullptr);

        m_layout->setContentsMargins(0, 0, 0, 0);

        buildToolbar();

        if (m_widget == nullptr) {
            auto* label = new QLabel("Select a panel type from the box on the top left.");
            label->setAlignment(Qt::AlignCenter);
            m_widget = label;
        }
        m_layout->addWidget(m_widget);
        m_layout->setContentsMargins(0, 0, 0, 0);

        setLayout(m_layout);
        setFrameShape(QFrame::Shape::StyledPanel);
    }

    class Panel* PanelWrapper::unwrap() {
        if (m_widget != nullptr) {
            m_widget->setParent(nullptr);
        }
        return qobject_cast<class Panel*>(std::exchange(m_widget, nullptr));
    }

    class Panel* PanelWrapper::get() const {
        return qobject_cast<class Panel*>(m_widget);
    }

    QComboBox* PanelWrapper::buildPanelSelector() {
        auto* panel_selector = new QComboBox;
        panel_selector->setPlaceholderText("<Select Panel>");
        panel_selector->setCurrentIndex(-1);

        QStringView target_id;
        if (auto* panel = get(); panel != nullptr) {
            const auto& metadata = panel->metadata();
            target_id            = metadata.id;
        }

        for (const auto& panel : panels::all) {
            panel_selector->addItem(panel.name.toString(), QVariant::fromValue(panel));
            if (panel.id == target_id) {
                panel_selector->setCurrentIndex(panel_selector->count() - 1);
            }
        }

        // TODO: move this to member function?
        connect(panel_selector, &QComboBox::currentIndexChanged, this, [this, panel_selector](int index) {
            if (m_widget != nullptr) {
                m_layout->removeWidget(m_widget);
                m_widget->deleteLater();
            }
            m_widget = std::invoke(panel_selector->itemData(index).value<PanelMetadata>().create, m_dataviewer);
            m_layout->addWidget(m_widget);
        });

        return panel_selector;
    }

    void PanelWrapper::buildToolbar() {
        auto* toolbar = new QToolBar;

        toolbar->addWidget(buildPanelSelector());
        toolbar->addSeparator();
        auto* split_hor_action = toolbar->addAction("Split Horizontally");
        auto* split_ver_action = toolbar->addAction("Split Vertically");
        auto* close_action     = toolbar->addAction("Close");

        connect(close_action, &QAction::triggered, this, &QWidget::deleteLater);
        connect(split_hor_action, &QAction::triggered, this, [this] {
            qCDebug(lc_layout) << "Split Horizontally";
            Q_EMIT requestedSplit(this, Qt::Horizontal);
        });
        connect(split_ver_action, &QAction::triggered, this, [this] {
            qCDebug(lc_layout) << "Split Vertically";
            Q_EMIT requestedSplit(this, Qt::Vertical);
        });

        m_layout->setMenuBar(toolbar);
    }

    PanelLayout::PanelLayout(DataViewer* dataviewer, Qt::Orientation orientation, PanelLayout* parent_layout,
                             QWidget* parent)
        : QSplitter(orientation, parent), m_dataviewer(dataviewer), m_parent_layout(parent_layout) {
        Q_ASSERT(dataviewer != nullptr);

        setChildrenCollapsible(false);
    }

    void PanelLayout::childEvent(QChildEvent* event) {
        Q_ASSERT(event != nullptr);

        QSplitter::childEvent(event);
        if (event->removed()) {
            if (auto remaining_widgets = count(); remaining_widgets == 0) {
                if (m_parent_layout == nullptr) {
                    addWidget();
                } else {
                    deleteLater();
                }
            } else if (remaining_widgets == 1 && m_parent_layout != nullptr) {
                qCDebug(lc_layout) << "Merging with parent layout. this:" << this
                                   << "parent:" << static_cast<void*>(m_parent_layout);
                auto* widget = this->widget(0);
                if (auto* panel_wrapper = qobject_cast<PanelWrapper*>(widget); panel_wrapper != nullptr) {
                    auto* panel = panel_wrapper->unwrap();
                    widget      = m_parent_layout->wrapPanel(panel);
                } else if (auto* layout = qobject_cast<PanelLayout*>(widget); layout != nullptr) {
                    layout->m_parent_layout = m_parent_layout;
                } else {
                    // This should only ever contain PanelWrappers or other PanelLayouts
                    Q_ASSERT(false);
                }
                auto  index    = m_parent_layout->indexOf(this);
                auto* this_ptr = m_parent_layout->replaceWidget(index, widget);
                Q_ASSERT(this_ptr == this);
                m_parent_layout = nullptr;
                deleteLater();
            }
        }
    }

    PanelLayout::~PanelLayout() {
        qCDebug(lc_layout) << "Destructed PanelLayout" << this;
    }

    void PanelLayout::addWidget(class Panel* panel) {
        qCDebug(lc_layout) << "Adding QWidget" << panel;
        addWidget(wrapPanel(panel));
    }

    void PanelLayout::addWidget(PanelWrapper* wrapper) {
        qCDebug(lc_layout) << "Adding PanelWrapper" << wrapper;
        Q_ASSERT(wrapper != nullptr);
        QSplitter::addWidget(wrapper);
    }

    void PanelLayout::insertWidget(int index, class Panel* panel) {
        qCDebug(lc_layout) << "Inserting widget" << panel << "at index" << index;
        QSplitter::insertWidget(index, wrapPanel(panel));
    };

    PanelWrapper* PanelLayout::wrapPanel(class Panel* panel) const {
        auto* wrapper = new PanelWrapper(m_dataviewer, panel);
        connect(wrapper, &PanelWrapper::requestedSplit, this, &PanelLayout::split);
        return wrapper;
    }

    void PanelLayout::split(PanelWrapper* wrapper, Qt::Orientation orientation) {
        Q_ASSERT(wrapper != nullptr);
        qCDebug(lc_layout) << "splitting widget" << wrapper;

        auto index = indexOf(wrapper);
        Q_ASSERT(index != -1);

        if (orientation == this->orientation()) {
            insertWidget(index + 1);
        } else {
            // TODO: Maintain panel_selector selection when reparenting into new layout
            auto* new_layout = new PanelLayout(m_dataviewer, orientation, this);
            auto* old        = qobject_cast<PanelWrapper*>(replaceWidget(index, new_layout));
            Q_ASSERT(old != nullptr);
            new_layout->addWidget(old->unwrap());
            new_layout->addWidget();
            old->deleteLater();
        }
    }

    QJsonDocument PanelLayout::saveState() const {
        return QJsonDocument{saveLayout()};
    }

    // NOLINTNEXTLINE(misc-no-recursion)
    QJsonObject PanelLayout::saveLayout() const {
        QJsonArray serialized_children;
        for (auto i = 0, total = count(); i < total; i++) {
            const auto* child = widget(i);
            if (const auto* panel_wrapper = qobject_cast<const PanelWrapper*>(child); panel_wrapper != nullptr) {
                if (auto* panel = panel_wrapper->get(); panel == nullptr) {
                    serialized_children.append(QJsonObject{
                        {"type", "empty"},
                    });
                } else {
                    serialized_children.append(QJsonObject({
                        {"type", "panel"                        },
                        {"id",   panel->metadata().id.toString()},
                        {"data", panel->saveState()             },
                    }));
                }
            } else if (const auto* layout = qobject_cast<const PanelLayout*>(child); layout != nullptr) {
                serialized_children.append(layout->saveLayout());
            }
        }
        return QJsonObject{
            {"type",           "layout"                                  },
            {"splitter_state", QString{QSplitter::saveState().toBase64()}},
            {"children",       std::move(serialized_children)            },
        };
    }

    bool PanelLayout::restoreState(const QJsonDocument& state) {
        qCDebug(lc_layout) << state;

        if (!state.isObject()) {
            qCDebug(lc_layout) << "restoreState: layout state must be an object";
            return false;
        }
        auto object = state.object();
        if (object["type"] != "layout") {
            qCDebug(lc_layout) << "restoreState: root object must represent a layout, got:" << object["type"];
            return false;
        }

        setUpdatesEnabled(false);

        auto res = restoreLayout(object);

        setUpdatesEnabled(true);

        return res;
    }

    // NOLINTNEXTLINE(misc-no-recursion,readability-function-cognitive-complexity)
    bool PanelLayout::restoreLayout(const QJsonObject& layout) {
        Q_ASSERT(layout["type"] == "layout");

        const auto& children = layout["children"];
        if (!children.isArray()) {
            qCDebug(lc_layout) << "restoreLayout: children needs to be an array, got:" << children.type();
            return false;
        }
        for (auto* child : this->children()) {
            child->deleteLater();
        }
        for (const auto& child : children.toArray()) {
            if (!child.isObject()) {
                qCDebug(lc_layout) << "restoreLayout: child needs to be an object, got:" << child.type();
                return false;
            }
            const auto& object     = child.toObject();
            const auto& child_type = object["type"];
            if (!child_type.isString()) {
                qCDebug(lc_layout) << "restoreLayout: child type needs to be a string, got:" << child_type.type();
                return false;
            }
            if (auto type = child_type.toString(); type == "layout") {
                auto* child_layout = new PanelLayout(m_dataviewer, {}, this);
                if (!child_layout->restoreLayout(object)) {
                    qCDebug(lc_layout) << "restoreLayout: failed to restore child layout";
                    return false;
                }
                QSplitter::addWidget(child_layout);
            } else if (type == "panel") {
                if (!restorePanel(object)) {
                    qCDebug(lc_layout) << "restoreLayout: failed to restore child panel";
                    return false;
                }
            } else if (type == "empty") {
                addWidget();
            } else {
                qCDebug(lc_layout) << "restoreLayout: Unknown type" << type;
                return false;
            }
        }

        const auto& splitter_state = layout["splitter_state"];
        if (!splitter_state.isString()) {
            qCDebug(lc_layout) << "restoreLayout: splitter_state needs to be a string, got:" << splitter_state.type();
            return false;
        }
        // TODO: figure out why this doesn't always restore layout exactly
        QSplitter::restoreState(QByteArray::fromBase64(splitter_state.toString().toUtf8()));

        return true;
    }

    bool PanelLayout::restorePanel(const QJsonObject& panel_data) {
        Q_ASSERT(panel_data["type"].toString() == "panel");

        const auto& panel_id = panel_data["id"];
        if (!panel_id.isString()) {
            qCDebug(lc_layout) << "restorePanel: id needs to be a string, got:" << panel_id.type();
            return false;
        }
        const auto& pid = panel_id.toString();

        for (const auto& metadata : panels::all) {
            if (pid != metadata.id) {
                continue;
            }

            auto* panel = std::invoke(metadata.create, m_dataviewer);
            if (panel == nullptr) {
                qCDebug(lc_layout) << "restorePanel: Failed creating a panel for id" << pid
                                   << "Adding empty panel instead.";
                addWidget();
                return true;
            }

            if (!panel_data["data"].isObject()) {
                qCDebug(lc_layout) << "restorePanel: data needs to be an object, got:" << panel_data["data"].type();
                return false;
            }
            if (!panel->restoreState(panel_data["data"].toObject())) {
                qCDebug(lc_layout) << "restorePanel: failed to restore panel data";
                return false;
            }

            addWidget(panel);
            return true;
        }

        qCDebug(lc_layout) << "restorePanel: Unknown panel id" << pid << "Adding empty panel instead.";
        addWidget();

        return true;
    }
} // namespace magnesia::activities::dataviewer::layout
