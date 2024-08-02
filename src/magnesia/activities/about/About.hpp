#pragma once

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "ConfigWidget.hpp"

#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::about {
    class About : public Activity {
        Q_OBJECT

      public:
        explicit About(QWidget* parent = nullptr);

      private:
        static const int s_header_size = 15;
        static const int s_title_size  = 20;
    };

    /**
     * `magnesia::ConfigWidget` for the `DataViewer` activity.
     */
    class ConfigWidget : public magnesia::ConfigWidget {
        Q_OBJECT

      public:
        explicit ConfigWidget(QWidget* parent = nullptr);

      private slots:
        void create();
    };

    inline constexpr ActivityMetadata metadata{
        .name                 = u"About",
        .global_init          = []() {},
        .create_config_widget = []() -> magnesia::ConfigWidget* { return new ConfigWidget; },
    };
} // namespace magnesia::activities::about
