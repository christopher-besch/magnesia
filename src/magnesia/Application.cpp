#include "Application.hpp"

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "Router.hpp"
#include "SQLStorageManager.hpp"
#include "SettingsManager.hpp"
#include "StorageManager.hpp"
#include "activities/activities.hpp"
#include "qt_version_check.hpp"
#include "settings.hpp"
#include "terminate.hpp"

#include <functional>
#include <memory>
#include <span>

#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#include <QtEnvironmentVariables>
#else
#include <QtGlobal>
#endif

namespace {
    Q_LOGGING_CATEGORY(lc_application, "magnesia")
} // namespace

namespace {
    bool use_debug() {
#ifndef NDEBUG
        bool debug = true;
#else
        bool debug = false;
#endif
        if (qEnvironmentVariableIsSet("MAGNESIA_DEBUG")) {
            debug = qEnvironmentVariableIntValue("MAGNESIA_DEBUG") != 0;
        }

        return debug;
    }

    QDir ensure_data_dir(bool debug) {
        QDir appdir;

        if (debug) {
            appdir = {"magnesia_data"};
        } else {
            appdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }

        if (!QDir{}.mkpath(appdir.absolutePath())) {
            qCCritical(lc_application) << "can't create data directory" << appdir.absolutePath();
            terminate();
        }

        qCInfo(lc_application) << "using data directory" << appdir.absolutePath();
        return appdir;
    }

    QString db_path(const QDir& appdir) {
        if (qEnvironmentVariableIsSet("MAGNESIA_EPHEMERAL_DB")) {
            return ":memory:";
        }
        return appdir.absoluteFilePath(QCoreApplication::applicationName() + ".db");
    }
} // namespace

namespace magnesia {
    Application* Application::s_instance{nullptr};

    Application::Application(QObject* parent)
        : QObject(parent), m_data_dir(ensure_data_dir(use_debug())),
          m_storage_manager(new SQLStorageManager{db_path(m_data_dir), this}),
          // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete): https://github.com/llvm/llvm-project/issues/62985
          m_settings_manager(new SettingsManager{m_storage_manager, this}), m_router(new Router{this}),
          m_tab_widget(new QTabWidget{&m_main_window}) {
        Q_ASSERT(s_instance == nullptr && "You can only have one mangesia::Application at a time");
        s_instance = this;

        m_settings_manager->defineSettingDomain(
            "general", {std::make_shared<magnesia::IntSetting>(
                           "opcua_poll_intervall", "OPC UA Polling Interval",
                           // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                           "in milliseconds; reload the application to apply", 500, 10, 30000)});

        m_tab_widget->setTabsClosable(true);
        m_tab_widget->setDocumentMode(true);
        connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, [this](int index) {
            auto* activity = m_tab_widget->widget(index);
            auto  closed   = activity->close();
            if (closed) {
                // NOTE: the cast will always be successful since we cast a value we know is the correct type, since we
                // passed it to the QTabWidget in `openActivity()`
                closeActivity(qobject_cast<Activity*>(activity));
            }
        });

        m_main_window.setCentralWidget(m_tab_widget);
        m_main_window.show();
    }

    Application::~Application() {
        Q_ASSERT(s_instance == this);
        s_instance = nullptr;
    }

    Application& Application::instance() {
        Q_ASSERT(s_instance != nullptr && "You need to create a magnesia::Application");
        return *s_instance;
    }

    void Application::initializeActivities() {
        for (const auto& activity : getActivityMetadata()) {
            if (activity.global_init != nullptr) {
                qCInfo(lc_application) << "initializing activity" << activity.name;
                std::invoke(activity.global_init);
            }
        }
    }

    std::span<const ActivityMetadata> Application::getActivityMetadata() {
        return activities::all;
    }

    StorageManager& Application::getStorageManager() {
        Q_ASSERT(m_storage_manager != nullptr);
        return *m_storage_manager;
    }

    SettingsManager& Application::getSettingsManager() {
        Q_ASSERT(m_settings_manager != nullptr);
        return *m_settings_manager;
    }

    Router& Application::getRouter() {
        Q_ASSERT(m_router != nullptr);
        return *m_router;
    }

    void Application::openActivity(Activity* activity, const QString& title, bool closable) {
        Q_ASSERT(activity != nullptr);
        Q_ASSERT(m_tab_widget != nullptr);

        // From https://doc.qt.io/qt-6/qtabwidget.html#addTab:
        // Note: If you call addTab() after show(), the layout system will try to adjust to the changes in its widgets
        // hierarchy and may cause flicker. To prevent this, you can set the QWidget::updatesEnabled property to false
        // prior to changes; remember to set the property to true when the changes are done, making the widget receive
        // paint events again.
        m_tab_widget->setUpdatesEnabled(false);
        auto index = m_tab_widget->addTab(activity, title);
        m_tab_widget->setCurrentIndex(index);

        if (!closable) {
            auto close_side = static_cast<QTabBar::ButtonPosition>(
                m_tab_widget->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, m_tab_widget));

            if (auto* close_button = m_tab_widget->tabBar()->tabButton(index, close_side); close_button != nullptr) {
                close_button->deleteLater();
                m_tab_widget->tabBar()->setTabButton(index, close_side, nullptr);
            }
        }

        m_tab_widget->setUpdatesEnabled(true);
    }

    void Application::closeActivity(Activity* activity) {
        Q_ASSERT(activity != nullptr);
        Q_ASSERT(m_tab_widget != nullptr);

        auto idx = m_tab_widget->indexOf(activity);
        Q_ASSERT(idx != -1);
        m_tab_widget->removeTab(idx);
        activity->deleteLater();
    }

    void Application::focusActivity(Activity* activity) {
        Q_ASSERT(activity != nullptr);
        Q_ASSERT(m_tab_widget != nullptr);

        auto idx = m_tab_widget->indexOf(activity);
        Q_ASSERT(idx != -1);
        m_tab_widget->setCurrentIndex(idx);
    }
} // namespace magnesia
