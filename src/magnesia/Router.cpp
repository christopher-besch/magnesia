#include "Router.hpp"

#include "qt_version_check.hpp"

#include <utility>

#include <QPointer>
#include <QString>
#include <QUrl>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia {
    void Router::registerDynamicSchemeHandler(const QString& scheme, QPointer<URLHandler> handler) {
        if (!handler.isNull()) {
            m_dynamic_handlers[scheme].push_back(std::move(handler));
        }
    }

    void Router::setStaticSchemeHandler(const QString& scheme, URLHandler* handler) {
        auto& existing_handler = m_static_handlers[scheme];
        if (existing_handler != nullptr) {
            existing_handler->deleteLater();
        }

        if (handler == nullptr) {
            m_static_handlers.erase(scheme);
        } else {
            handler->setParent(this);
            existing_handler = handler;
        }
    }

    void Router::setFallbackHandler(URLHandler* handler) {
        if (m_fallback_handler != nullptr) {
            m_fallback_handler->deleteLater();
        }
        if (handler != nullptr) {
            handler->setParent(this);
        }
        m_fallback_handler = handler;
    }

    bool Router::route(const QUrl& url) {
        auto scheme = url.scheme();

        if (auto dynamic_handlers = m_dynamic_handlers.find(scheme); dynamic_handlers != m_dynamic_handlers.end()) {
            // Remove destroyed handlers. QPointer becomes null when it's object is destroyed.
            dynamic_handlers->second.removeIf([](const QPointer<URLHandler>& handler) { return handler.isNull(); });
            for (const auto& handler : dynamic_handlers->second) {
                auto res = handler->handleURL(url);
                if (res) {
                    return true;
                }
            }
        }

        if (auto static_handler = m_static_handlers.find(scheme); static_handler != m_static_handlers.end()) {
            Q_ASSERT(static_handler->second != nullptr);
            auto res = static_handler->second->handleURL(url);
            if (res) {
                return true;
            }
        }

        if (m_fallback_handler != nullptr) {
            auto res = m_fallback_handler->handleURL(url);
            if (res) {
                return true;
            }
        }

        return false;
    }
} // namespace magnesia
