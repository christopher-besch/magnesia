#pragma once

#include <cstdint>

#include <open62541pp/open62541pp.h>
#include <open62541pp/services/MonitoredItem.h>
#include <open62541pp/types/Builtin.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Parameters for a MonitoredItem
     *
     * @see MonitoredItem
     *
     * See https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
     */
    class MonitoringParameters {
      public:
        explicit MonitoringParameters(opcua::services::MonitoringParametersEx parameters);

        /**
         * Set the sampling interval. It determines how often to look for new changes of this item.
         *
         * @param interval the new sampling interval
         */
        void setSamplingInterval(double interval) noexcept;

        /**
         * Set the size of the monitoring queue.
         *
         * @param size the new queue size
         */
        void setQueueSize(uint32_t size) noexcept;

        /**
         * Set the discard policy for when the queue is full. If true the oldest item will be discarded. Otherwise the
         * newest will be discarded.
         *
         * @param discard the new discard policy
         */
        void setDiscardOldest(bool discard) noexcept;

        /**
         * Get the sampling interval.
         */
        [[nodiscard]] double getSamplingInterval() const noexcept;
        /**
         * Get the queue size.
         */
        [[nodiscard]] uint32_t getQueueSize() const noexcept;

        /**
         * Get the discard policy.
         */
        [[nodiscard]] bool getDiscardOldest() const noexcept;

        /**
         * Get the underlying monitoring parameters.
         */
        [[nodiscard]] const opcua::services::MonitoringParametersEx& handle() const noexcept;

        /**
         * Get the underlying monitoring parameters.
         */
        [[nodiscard]] opcua::services::MonitoringParametersEx& handle() noexcept;

      private:
        opcua::services::MonitoringParametersEx m_parameters;
    };
} // namespace magnesia::opcua_qt::abstraction
