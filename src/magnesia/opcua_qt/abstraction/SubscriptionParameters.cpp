#include "SubscriptionParameters.hpp"

#include <cstdint>

#include <open62541pp/Subscription.h>
#include <open62541pp/services/Subscription.h>

namespace magnesia::opcua_qt::abstraction {
    SubscriptionParameters::SubscriptionParameters(opcua::services::SubscriptionParameters parameters)
        : m_parameters(parameters) {}

    void SubscriptionParameters::setPublishingInterval(double interval) noexcept {
        m_parameters.publishingInterval = interval;
    }

    void SubscriptionParameters::setLifetimeCount(std::uint32_t count) noexcept {
        m_parameters.lifetimeCount = count;
    }

    void SubscriptionParameters::setMaxKeepAliveCount(std::uint32_t count) noexcept {
        m_parameters.maxKeepAliveCount = count;
    }

    void SubscriptionParameters::setMaxNotificationsPerPublish(std::uint32_t count) noexcept {
        m_parameters.maxNotificationsPerPublish = count;
    }

    void SubscriptionParameters::setPriority(std::uint8_t priority) noexcept {
        m_parameters.priority = priority;
    }

    [[nodiscard]] double SubscriptionParameters::getPublishingInterval() const noexcept {
        return m_parameters.publishingInterval;
    }

    [[nodiscard]] std::uint32_t SubscriptionParameters::getLifetimeCount() const noexcept {
        return m_parameters.lifetimeCount;
    }

    [[nodiscard]] std::uint32_t SubscriptionParameters::getMaxKeepAliveCount() const noexcept {
        return m_parameters.maxKeepAliveCount;
    }

    [[nodiscard]] std::uint32_t SubscriptionParameters::getMaxNotificationsPerPublish() const noexcept {
        return m_parameters.maxNotificationsPerPublish;
    }

    [[nodiscard]] std::uint8_t SubscriptionParameters::getPriority() const noexcept {
        return m_parameters.priority;
    }

    [[nodiscard]] const opcua::SubscriptionParameters& SubscriptionParameters::handle() const noexcept {
        return m_parameters;
    }

    [[nodiscard]] opcua::SubscriptionParameters& SubscriptionParameters::handle() noexcept {
        return m_parameters;
    }
} // namespace magnesia::opcua_qt::abstraction
