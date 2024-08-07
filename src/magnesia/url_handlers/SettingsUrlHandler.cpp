#include "SettingsUrlHandler.hpp"

#include "../Activity.hpp"
#include "../Application.hpp"
#include "../activities/settings_activity/SettingsActivity.hpp"

#include <QUrl>
#include <QUrlQuery>

namespace magnesia::url_handlers {
    SettingsUrlHandler::SettingsUrlHandler(activities::settings_activity::SettingsActivity* activity)
        : m_activity(activity) {}

    bool SettingsUrlHandler::handleURL(const QUrl& url) {
        auto query = QUrlQuery{url};
        if (url.path() == "focus-domain") {
            return m_activity->focusDomain(query.queryItemValue("domain"));
        }
        if (url.path() == "focus-setting") {
            return m_activity->focusSetting({query.queryItemValue("name"), query.queryItemValue("domain")});
        }
        if (url.path() == "focus") {
            Application::instance().focusActivity(dynamic_cast<Activity*>(this));
            return true;
        }
        return false;
    }
} // namespace magnesia::url_handlers
