#include "SettingsUrlHandler.hpp"

#include "../../Application.hpp"
#include "../../Router.hpp"
#include "Settings.hpp"

#include <QUrl>
#include <QUrlQuery>

namespace magnesia::activities::settings {
    SettingsUrlHandler::SettingsUrlHandler(Settings* activity) : URLHandler(activity), m_activity(activity) {}

    bool SettingsUrlHandler::handleURL(const QUrl& url) {
        const QUrlQuery query{url};
        auto            path = url.path();
        if (path == "focus-domain") {
            m_activity->focusDomain(query.queryItemValue("domain"));
        } else if (path == "focus-setting") {
            m_activity->focusSetting({.name = query.queryItemValue("name"), .domain = query.queryItemValue("domain")});
        } else if (path == "focus") {
            Application::instance().focusActivity(m_activity);
        } else {
            return false;
        }
        return true;
    }
} // namespace magnesia::activities::settings
