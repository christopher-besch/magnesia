#pragma once

#include <qtmetamacros.h>
#include <QWidget>

namespace magnesia {
    /**
     * Base class for implementing new activities.
     *
     * @see ActivityMetadata
     * @see ConfigWidget
     */
    class Activity : public QWidget {
        Q_OBJECT

      public:
        using QWidget::QWidget;
    };
} // namespace magnesia
