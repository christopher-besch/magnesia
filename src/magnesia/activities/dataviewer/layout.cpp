#include "layout.hpp"

#include "activities/dataviewer/DataViewer.hpp"
#include "activities/dataviewer/PanelMetadata.hpp"
#include "activities/dataviewer/panels.hpp"
#include "qt_version_check.hpp"

#include <functional>
#include <utility>

#include <QAction>
#include <QChildEvent>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QObject>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtLogging>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::layout {
    PanelWrapper::PanelWrapper(DataViewer* dataviewer, QWidget* widget, QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout), m_widget(widget), m_dataviewer(dataviewer) {
        Q_ASSERT(dataviewer != nullptr);

        buildToolbar();

        if (m_widget == nullptr) {
            auto* label = new QLabel("Select a panel type from the box on the top left.");
            label->setAlignment(Qt::AlignCenter);
            m_widget = label;
        }
        m_layout->addWidget(m_widget, 0, Qt::AlignCenter);

        setLayout(m_layout);
        setFrameShape(QFrame::Shape::Box);
    }

    QWidget* PanelWrapper::unwrap() {
        if (m_widget != nullptr) {
            m_widget->setParent(nullptr);
        }
        return std::exchange(m_widget, nullptr);
    }

    QComboBox* PanelWrapper::buildPanelSelector() {
        auto* panel_selector = new QComboBox;
        for (const auto& panel : panels::all) {
            panel_selector->addItem(panel.name.toString(), QVariant::fromValue(panel));
        }
        panel_selector->setPlaceholderText("<Select Panel>");
        panel_selector->setCurrentIndex(-1);

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
        connect(split_hor_action, &QAction::triggered, this, [this]() {
            qDebug() << "Split Horizontally";
            Q_EMIT requestedSplit(this, Qt::Horizontal);
        });
        connect(split_ver_action, &QAction::triggered, this, [this]() {
            qDebug() << "Split Vertically";
            Q_EMIT requestedSplit(this, Qt::Vertical);
        });

        auto* separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);

        m_layout->addWidget(toolbar);
        m_layout->addWidget(separator);
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
                qDebug() << "PanelLayout: Merging with parent layout. this:" << this
                         << "parent:" << static_cast<void*>(m_parent_layout);
                auto* widget = this->widget(0);
                if (auto* panel_wrapper = qobject_cast<PanelWrapper*>(widget); panel_wrapper != nullptr) {
                    widget = panel_wrapper->unwrap();
                    widget = m_parent_layout->wrapWidget(widget);
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
        qDebug() << "Destructed PanelLayout" << this;
    }

    void PanelLayout::addWidget(QWidget* widget) {
        qDebug() << "PanelLayout: Adding QWidget" << widget;
        addWidget(wrapWidget(widget));
    }

    void PanelLayout::addWidget(PanelWrapper* widget) {
        qDebug() << "PanelLayout: Adding PanelWrapper" << widget;
        Q_ASSERT(widget != nullptr);
        QSplitter::addWidget(widget);
    }

    void PanelLayout::insertWidget(int index, QWidget* widget) {
        qDebug() << "PanelLayout: Inserting widget" << widget << "at index" << index;
        QSplitter::insertWidget(index, wrapWidget(widget));
    };

    PanelWrapper* PanelLayout::wrapWidget(QWidget* widget) const {
        auto* wrapper = new PanelWrapper(m_dataviewer, widget);
        connect(wrapper, &PanelWrapper::requestedSplit, this, &PanelLayout::split);
        return wrapper;
    }

    void PanelLayout::split(PanelWrapper* widget, Qt::Orientation orientation) {
        Q_ASSERT(widget != nullptr);
        qDebug() << "PanelLayout: splitting widget" << widget;

        auto index = indexOf(widget);
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
} // namespace magnesia::activities::dataviewer::layout
