#pragma once

#include "MonitoringMode.hpp"
#include "MonitoringParameters.hpp"
#include "open62541pp/Client.h"
#include "open62541pp/MonitoredItem.h"

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class MonitoredItem
     * @brief Represents an attribute or an event that was subscribed to.
     *
     * @see Subscription
     *
     * See https://reference.opcfoundation.org/Core/Part1/v105/docs/3.2.20
     */
    class MonitoredItem {
      public:
        /**
         * @param item Client to be monitored
         */
        explicit MonitoredItem(opcua::MonitoredItem<opcua::Client> item);

        /**
         * Set a new monitoring mode.
         *
         * @see MonitoringMode
         *
         * @param mode the new monitoring mode
         */
        void setMonitoringMode(MonitoringMode mode);

        /**
         * Set new monitoring parameters.
         *
         * @see MonitoringParameters
         *
         * @param parameters the new monitoring parameters
         */
        void setMonitoringParameters(MonitoringParameters parameters);

        /**
         * Get the underlying monitored item.
         */
        [[nodiscard]] const opcua::MonitoredItem<opcua::Client>& handle() const noexcept;

        /**
         * Get the underlying monitored item.
         */
        [[nodiscard]] opcua::MonitoredItem<opcua::Client>& handle() noexcept;

      private:
        opcua::MonitoredItem<opcua::Client> m_item;
    };
} // namespace magnesia::opcua_qt::abstraction
