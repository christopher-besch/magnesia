#pragma once

#include <QUrl>
#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia {
    /**
     * Activity configuration UI shown in the AddActivity Activity Tab.
     *
     * @see Activity
     * @see ActivityMetadata
     */
    class ConfigWidget : public QWidget {
        Q_OBJECT

      public:
        using QWidget::QWidget;

        /**
         * Classed by the AddActivity Activity to handle OPC UA URLs. The common use case is to pre-fill the UI and
         * prompt the user for the rest. The URL is passed to all `ConfigWidget`s, so the handler should not
         * automatically create any activities to prevent launching multiple activities.
         *
         * The default implementation discards the URL and returns false.
         *
         * @param url The URL to handle.
         *
         * @return Whether this widget accepted the URL.
         */
        virtual bool handleURL(const QUrl& url);
    };
} // namespace magnesia
