#pragma once

#include "../../Router.hpp"
#include "Settings.hpp"

#include <QUrl>

namespace magnesia::activities::settings {
    /**
     * @class SettingsUrlHandler
     * @brief URLHandler for the settings activity.
     */
    class SettingsUrlHandler : public URLHandler {
      public:
        /**
         * @param activity Settings activity opening the url handler.
         */
        explicit SettingsUrlHandler(Settings* activity);
        bool handleURL(const QUrl& url) override;

      private:
        Settings* m_activity{nullptr};
    };
} // namespace magnesia::activities::settings
