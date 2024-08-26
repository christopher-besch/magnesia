#pragma once

#include "../../Router.hpp"

#include <QUrl>

namespace magnesia::activities::certificate {
    /**
     * @class CertificateURLHandler
     * @brief URLHandler for the certificate activity.
     */
    class CertificateURLHandler : public URLHandler {
      public:
        bool handleURL(const QUrl& url) override;
    };
} // namespace magnesia::activities::certificate
