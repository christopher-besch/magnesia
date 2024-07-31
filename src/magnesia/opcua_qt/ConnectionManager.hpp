#pragma once

#include "Connection.hpp"
#include "ConnectionBuilder.hpp"

#include <QList>
#include <QMap>
#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt {
    /**
     * @class ConnectionManager
     * @brief The ConnectionManager is parent to all connections and manages them.
     */
    class ConnectionManager : public QObject {
        Q_OBJECT

      public:
        explicit ConnectionManager(QObject* parent = nullptr);

        ConnectionManager(const ConnectionManager&)            = delete;
        ConnectionManager(ConnectionManager&&)                 = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;
        ConnectionManager& operator=(ConnectionManager&&)      = delete;

        ~ConnectionManager() override;
        /**
         * @brief Gets the connection corresponding to that id.
         * Returns nullptr if there is no Connection with that id.
         *
         * @param connection_id id of the connection
         * @return pointer to the Connection Object
         */
        [[nodiscard]] Connection* getConnection(int connection_id) noexcept;
        /**
         * @brief Gets a List of all Connection Ids
         *
         * @return QList of ids.
         */
        [[nodiscard]] QList<int> getConnectionIds() const noexcept;
        /**
         * @brief Disconnects the connection, if it is open.
         * Does nothing if the id is unused.
         *
         * @param connection_id id of the connection
         */
        void closeConnection(int connection_id);
        /**
         * @brief Closes all containing connections.
         */
        void closeAllConnections();

      private:
        friend ConnectionBuilder;

        /**
         * consume the ConnectionBuilder
         */
        [[nodiscard]] Connection* createConnection(ConnectionBuilder& connection_builder);

      private:
        QMap<int, Connection*> m_connections;
        int                    m_last_connection_id{};
    };
} // namespace magnesia::opcua_qt
