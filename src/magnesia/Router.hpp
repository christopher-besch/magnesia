#pragma once

#include <map>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <qtmetamacros.h>

namespace magnesia {
    /**
     * Interface to implement custom URL handlers for the `Router` class.
     */
    class URLHandler : public QObject {
        Q_OBJECT

      public:
        using QObject::QObject;

        /**
         * Called to handle a url.
         *
         * @param url The URL to handle.
         * @return Whether the URL was handled.
         */
        virtual bool handleURL(const QUrl& url) = 0;
    };

    class Router : public QObject {
        Q_OBJECT

      public:
        using QObject::QObject;

        /**
         * Registers a `URLHandler` for a scheme in addition to the existing ones.
         *
         * @note This function does _not_ take ownership of the handler it is passed. The handler is automatically
         * deregistered when it is destructed.
         * @param scheme The scheme to register a handler for.
         * @param handler A `QPointer` to the handler to register
         */
        void registerDynamicSchemeHandler(const QString& scheme, QPointer<URLHandler> handler);

        /**
         * Every URL scheme can have one static `URLHandler` used when no dynamic handler handled a URL.
         * This sets that handler for a specific scheme. This replaces the previously set handler.
         * Passing `nullptr` for the handler removes the static handler for that scheme.
         *
         * @note This function takes ownership over the handler and destroys it when necessary
         * @param scheme The scheme to set the handler for.
         * @param handler The handler to use.
         */
        void setStaticSchemeHandler(const QString& scheme, URLHandler* handler);

        /**
         * If no dynamic or static `URLHandler` managed to handle a URL the global fallback handler is used.
         * This sets that global fallback handler. This replaces the previously set handler.
         * Passing `nullptr` removes the handler.
         *
         * @note This function takes ownership over the handler and destroys it when necessary
         * @param handler The handler to set.
         */
        void setFallbackHandler(URLHandler* handler);

        /**
         * Routes a URL to the appropriate handler. It iterates through the registered dynamic, static and fallback
         * handlers and stops at the first one that returns true, indicating it handled the URL.
         *
         * @param url The URL to route.
         * @return Whether any handler handled the URL.
         */
        bool route(const QUrl& url);

      private:
        std::map<QString, std::vector<QPointer<URLHandler>>> m_dynamic_handlers;
        std::map<QString, URLHandler*>                       m_static_handlers;
        URLHandler*                                          m_fallback_handler{};
    };
} // namespace magnesia
