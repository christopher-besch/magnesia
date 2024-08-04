#pragma once

#include "Activity.hpp"
#include "ActivityMetadata.hpp"
#include "Router.hpp"
#include "SettingsManager.hpp"
#include "StorageManager.hpp"
#include "opcua_qt/ConnectionManager.hpp"
#include "qt_version_check.hpp"

#include <span>

#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QTabWidget>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtClassHelperMacros>
#else
#include <QtGlobal>
#endif

namespace magnesia {
    /**
     * A sort-of singleton managing the application state and main window and providing the necessary APIs to the
     * activities. The singleton design is similar to Qt's `QCoreApplication` with the user creating and owning an
     * instance, but that instance being available globally using the `Application::instance()` function. This allows to
     * create multiple, temporally non-overlapping instances during one program execution.
     *
     * @note Creating multiple instances that (partially) exist at the same time is not allowed and terminates the
     * program.
     */
    class Application : public QObject {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Application)

        /**
         * Global pointer to the current instance.
         */
        static Application* s_instance;

      public:
        /**
         * Sets the global instance to the constructed object and initializes the main window and the persistence and
         * OPC UA subsystems.
         */
        explicit Application(QObject* parent = nullptr);
        /**
         * Unsets the global instance.
         */
        ~Application() override;

        /**
         * Accessor for the current global instance.
         *
         * @note Calling this while there is no instance is not allowed and terminates the program.
         */
        static Application& instance();

        /**
         * Loops through all the `ActivityMetadata` from `getActivityMetadata()` and calls their `global_init` member if
         * it is not null.
         */
        static void initializeActivities();

        /**
         * Provides the list of registered activities.
         */
        [[nodiscard]] static std::span<const ActivityMetadata> getActivityMetadata();

        /**
         * Provides a reference to the current `StorageManager` that should be used by activities to store/retrieve
         * data.
         */
        StorageManager& getStorageManager();

        /**
         * Provides a reference to the current `SettingsManager` that should be used by activities to store/retrieve
         * settings.
         */
        SettingsManager& getSettingsManager();

        /**
         * Provides a reference to the current `ConnectionManager` that should be used by activities to create and
         * manage OPC UA connections.
         */
        opcua_qt::ConnectionManager& getConnectionManager();

        /**
         * Provides a reference to the current `Router` that should be used by activities to route messages to the
         * relevant parties.
         */
        Router& getRouter();

        /**
         * Opens a new tab that contains the activity.
         *
         * @note Calling this with an `Activity` that has already been opened invokes undefined behavior.
         *
         * @param activity the activity to display
         * @param title the title of the new tab
         * @param closable whether a close button should be shown next to the tab title
         */
        void openActivity(Activity* activity, const QString& title, bool closable = true);

        /**
         * Closes the tab that contains the activity.
         *
         * @note Calling this with an `Activity` that was not passed to `openActivity()` or that has already been closed
         * is not allowed and terminates the program.
         *
         * @param activity the activity whose tab should be closed
         */
        void closeActivity(Activity* activity);

        /**
         * Focuses the tab that contains the activity.
         *
         * @note Calling this with an `Activity` that is not currently open is not allowed and terminates the program.
         *
         * @param activity the activity whose tab should be focused
         */
        void focusActivity(Activity* activity);

      private:
        StorageManager*              m_storage_manager{nullptr};
        SettingsManager*             m_settings_manager{nullptr};
        opcua_qt::ConnectionManager* m_connection_manager{nullptr};
        Router*                      m_router{nullptr};

        // Not a pointer to maintain ownership. QMainWindow doesn't accept a QObject pointer (this) as parent so the
        // QObject tree doesn't destruct this when the Application is destroyed, leaving a bunch of stuff behind that
        // tries to access instance().
        QMainWindow m_main_window;
        QTabWidget* m_tab_widget{nullptr};
    };
} // namespace magnesia
