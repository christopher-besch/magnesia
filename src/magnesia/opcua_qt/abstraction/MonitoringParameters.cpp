#include "MonitoringParameters.hpp"

#include <cstdint>
#include <utility>

#include <open62541pp/services/MonitoredItem.h>

namespace magnesia::opcua_qt::abstraction {
    MonitoringParameters::MonitoringParameters(opcua::services::MonitoringParametersEx parameters)
        : m_parameters(std::move(parameters)) {}

    void MonitoringParameters::setSamplingInterval(double interval) noexcept {
        m_parameters.samplingInterval = interval;
    }

    void MonitoringParameters::setQueueSize(std::uint32_t size) noexcept {
        m_parameters.queueSize = size;
    }

    void MonitoringParameters::setDiscardOldest(bool discard) noexcept {
        m_parameters.discardOldest = discard;
    }

    [[nodiscard]] double MonitoringParameters::getSamplingInterval() const noexcept {
        return m_parameters.samplingInterval;
    }

    [[nodiscard]] std::uint32_t MonitoringParameters::getQueueSize() const noexcept {
        return m_parameters.queueSize;
    }

    [[nodiscard]] bool MonitoringParameters::getDiscardOldest() const noexcept {
        return m_parameters.discardOldest;
    }

    [[nodiscard]] const opcua::services::MonitoringParametersEx& MonitoringParameters::handle() const noexcept {
        return m_parameters;
    }

    [[nodiscard]] opcua::services::MonitoringParametersEx& MonitoringParameters::handle() noexcept {
        return m_parameters;
    }
} // namespace magnesia::opcua_qt::abstraction
