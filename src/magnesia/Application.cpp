#include "Application.hpp"

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "activities/activities.hpp"
// TODO: uncomment when ConnectionManager is available
// #include "ConnectionManager.hpp"
#include "Router.hpp"
#include "SQLStorageManager.hpp"
#include "SettingsManager.hpp"
#include "StorageManager.hpp"
#include "qt_version_check.hpp"

#include <functional>
#include <span>

#include <QObject>
#include <QString>
#include <QTabWidget>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia {
    Application* Application::s_instance{nullptr};

    Application::Application(QObject* parent)
        : QObject(parent),
          // TODO: use persistent db instead of :memory:
          m_storage_manager(new SQLStorageManager{":memory:", this}),
          // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete): https://github.com/llvm/llvm-project/issues/62985
          m_settings_manager(new SettingsManager{m_storage_manager, this}),
          // TODO: uncomment when ConnectionManager is available
          // m_connection_manager{new ConnectionManager{this}},
          m_router(new Router{this}), m_tab_widget(new QTabWidget{&m_main_window}) {
        Q_ASSERT(s_instance == nullptr && "You can only have one mangesia::Application at a time");
        s_instance = this;

        m_tab_widget->setTabsClosable(true);
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

    // TODO: uncomment when ConnectionManager is available
    // ConnectionManager& Application::getConnectionManager() {
    //     Q_ASSERT(m_connection_manager != nullptr);
    //     return *m_connection_manager;
    // }

    Router& Application::getRouter() {
        Q_ASSERT(m_router != nullptr);
        return *m_router;
    }

    void Application::openActivity(Activity* activity, const QString& title) {
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
