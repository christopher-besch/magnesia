#pragma once

#include <cstdint>

#include <open62541pp/services/Subscription.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Holds parameters for a Subscription.
     *
     * @see Subscription
     */
    class SubscriptionParameters {
      public:
        explicit SubscriptionParameters(opcua::services::SubscriptionParameters parameters);

        /**
         * Set the minimum time between publishes in milliseconds.
         */
        void setPublishingInterval(double interval) noexcept;

        /**
         * Set the lifetime of the subscription. After the defined number of publishing cycles without sending any
         * notifications the subscription will be discarded.
         */
        void setLifetimeCount(uint32_t count) noexcept;

        /**
         * Set the max keep alive count. After the defined number of publishing cycles without sending any
         * notifications a keep alive message will be sent.
         */
        void setMaxKeepAliveCount(uint32_t count) noexcept;

        /**
         * Set the maximum number of notifications that will be sent per publish. 0 means unlimited.
         */
        void setMaxNotificationsPerPublish(uint32_t count) noexcept;

        /**
         * Set the priority of the subscription. Notifications with higher priority are sent first.
         */
        void setPriority(uint8_t priority) noexcept;

        /**
         * Get the publishing interval.
         */
        [[nodiscard]] double getPublishingInterval() const noexcept;

        /**
         * Get the lifetime count.
         */
        [[nodiscard]] uint32_t getLifetimeCount() const noexcept;

        /**
         * Get the maximum keep alive count.
         */
        [[nodiscard]] uint32_t getMaxKeepAliveCount() const noexcept;

        /**
         * Get the maximum number of notifications per publish.
         */
        [[nodiscard]] uint32_t getMaxNotificationsPerPublish() const noexcept;

        /**
         * Get the priority of the subscription.
         */
        [[nodiscard]] uint8_t getPriority() const noexcept;

        /**
         * Get the underlying subscription parameters.
         */
        [[nodiscard]] const opcua::services::SubscriptionParameters& handle() const noexcept;

        /**
         * Get the underlying subscription parameters.
         */
        [[nodiscard]] opcua::services::SubscriptionParameters& handle() noexcept;

      private:
        opcua::services::SubscriptionParameters m_parameters;
    };
} // namespace magnesia::opcua_qt::abstraction
