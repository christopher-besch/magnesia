#pragma once

#include "../Router.hpp"
#include "../activities/settings_activity/SettingsActivity.hpp"

#include <QUrl>

namespace magnesia::url_handlers {
    class SettingsUrlHandler : public URLHandler {
      public:
        explicit SettingsUrlHandler(activities::settings_activity::SettingsActivity* activity);
        bool handleURL(const QUrl& url) override;

      private:
        activities::settings_activity::SettingsActivity* m_activity{nullptr};
    };
} // namespace magnesia::url_handlers
