#pragma once

#include "../../Activity.hpp"
#include "../../ActivityMetadata.hpp"
#include "../../ConfigWidget.hpp"

#include <QWidget>
#include <qtmetamacros.h>

namespace magnesia::activities::about {
    /**
     * @class About
     * @brief About activity for the application.
     */
    class About : public Activity {
        Q_OBJECT

      public:
        /**
         * @param parent Parent of the about page.
         */
        explicit About(QWidget* parent = nullptr);

      private:
        static constexpr int s_header_size = 15;
        static constexpr int s_title_size  = 20;
    };

    /**
     * @class ConfigWidget
     * @brief `magnesia::ConfigWidget` for the `About` activity.
     */
    class ConfigWidget : public magnesia::ConfigWidget {
        Q_OBJECT

      public:
        explicit ConfigWidget(QWidget* parent = nullptr);
    };

    inline constexpr ActivityMetadata metadata{
        .name                 = u"About",
        .create_config_widget = []() -> magnesia::ConfigWidget* { return new ConfigWidget; },
    };
} // namespace magnesia::activities::about
