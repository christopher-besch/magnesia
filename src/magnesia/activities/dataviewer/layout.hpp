#pragma once

#include "../../qt_version_check.hpp"
#include "dataviewer_fwd.hpp"

#include <QChildEvent>
#include <QComboBox>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtClassHelperMacros>
#else
#include <QtGlobal>
#endif

namespace magnesia::activities::dataviewer::layout {
    /**
     * @class PanelWrapper
     * @brief Helper class that wraps a `QWidget` in a panel with a toolbar to add the layout actions and a selector for
     * the type of panel to display.
     */
    class PanelWrapper : public QFrame {
        Q_OBJECT

      public:
        /**
         * Builds a (potentially empty) panel.
         *
         * @param dataviewer `DataViewer` to pass to panels `PanelMetadata::create` function.
         * @param panel a widget to wrap. An empty panel is created if this parameter is nullptr or omitted.
         * @param parent the parent QWidget passed to Qt.
         */
        explicit PanelWrapper(DataViewer* dataviewer, class Panel* panel = nullptr, QWidget* parent = nullptr);

        /**
         * Extracts the wrapped widget. This passes ownership of the widget to the caller and removes the
         * parent/child relationship with this object.
         *
         * @returns the contained `QWidget`.
         */
        class Panel* unwrap();

        /**
         * Returns the wrapped widget. This does not change the ownership over the returned widget.
         *
         * @returns the contained `QWidget`.
         */
        [[nodiscard]] class Panel* get() const;

      signals:
        /**
         * Emitted to signal that the widget should be split by the parent `PanelLayout`.
         *
         * @param wrapper the `PanelWrapper` to split. Usually `this`.
         * @param orientation the direction in which to split.
         */
        void requestedSplit(PanelWrapper* wrapper, Qt::Orientation orientation);

      private:
        /**
         * Helper that builds a `QComboBox` for the user to select the panel type.
         *
         * @returns the selector
         */
        QComboBox* buildPanelSelector();
        /**
         * Helper that builds the toolbar containing the layout actions.
         */
        void buildToolbar();

      private:
        QVBoxLayout* m_layout{nullptr};
        QWidget*     m_widget{nullptr};

        DataViewer* m_dataviewer{nullptr};
    };

    /**
     * @class PanelLayout
     * @brief Provides a 2D panel layouting system for the `DataViewer`.
     */
    class PanelLayout : public QSplitter {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(PanelLayout)

      public:
        /**
         * Constructor.
         *
         * @param dataviewer `DataViewer` to pass to panels `PanelMetadata::create` function.
         * @param orientation the base orientation of the layout. Directly passed to the underlying `QSplitter`.
         * @param parent_layout the parent `PanelLayout` or nullptr if this is the root.
         * @param parent the parent QWidget passed to Qt.
         */
        explicit PanelLayout(DataViewer* dataviewer, Qt::Orientation orientation, PanelLayout* parent_layout,
                             QWidget* parent = nullptr);
        ~PanelLayout() override;
        /**
         * Wraps a widget in a `PanelWrapper` and appends it to the end of the layout.
         *
         * @param panel the widget to add
         */
        void addWidget(class Panel* panel = nullptr);
        /**
         * Appends a wrapped widget to the end of the layout.
         *
         * @param wrapper the wrapped widget to add
         */
        void addWidget(PanelWrapper* wrapper);
        /**
         * Wraps a widget in a `PanelWrapper` and inserts it at the specified index.
         *
         * @param index the index where to add the widget
         * @param panel the widget to add
         */
        void insertWidget(int index, class Panel* panel = nullptr);

        /**
         * Saves the current state as Layout.
         *
         * @return JSON document containing the layout.
         */
        [[nodiscard]] QJsonDocument saveState() const;

        /**
         * Restores the former state of panels stored as Layout.
         *
         * @return true iff restoring the state succeeded.
         */
        bool restoreState(const QJsonDocument& state);

      protected:
        /**
         * Reimplements `QSplitter::childEvent`.
         *
         * Merges empty or unnecessary layouts into their parent if available, or adds an empty `PanelWrapper` when the
         * last child was removed if this is the root layout.
         */
        void childEvent(QChildEvent* event) override;

      private:
        /**
         * Helper that wraps a widget into a `PanelWrapper`.
         *
         * @param panel the widget to wrap
         * @returns the wrapped widget
         */
        [[nodiscard]] PanelWrapper* wrapPanel(class Panel* panel) const;

        [[nodiscard]] QJsonObject saveLayout() const;
        [[nodiscard]] bool        restoreLayout(const QJsonObject& layout);
        [[nodiscard]] bool        restorePanel(const QJsonObject& panel);

      private slots:
        /**
         * Splits a panel in the specified orientation, adding a child layout if necessary.
         *
         * @param wrapper the `PanelWrapper` to split. Usually `this`.
         * @param orientation the direction in which to split.
         */
        void split(PanelWrapper* wrapper, Qt::Orientation orientation);

      private:
        DataViewer*  m_dataviewer{nullptr};
        PanelLayout* m_parent_layout{nullptr};
    };
} // namespace magnesia::activities::dataviewer::layout
