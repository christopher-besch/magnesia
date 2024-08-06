#include "CertificateURLHandler.hpp"

#include "../../Application.hpp"
#include "Certificate.hpp"

#include <optional>

#include <QUrl>
#include <QUrlQuery>

namespace magnesia::activities::certificate {
    bool CertificateURLHandler::handleURL(const QUrl& url) {
        auto path = url.path();

        if (path == "create") {
            Application::instance().openActivity(new Certificate(std::nullopt), "Create Certificate");
        } else if (path == "view") {
            const QUrlQuery query{url};
            auto            storage_id = query.queryItemValue("storage-id");
            if (storage_id.isEmpty()) {
                return false;
            }
            Application::instance().openActivity(new Certificate(storage_id.toULongLong()), "View Certificate");
        } else {
            return false;
        }

        return true;
    }
} // namespace magnesia::activities::certificate
