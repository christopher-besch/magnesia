#include "MonitoredItem.hpp"

#include "MonitoringMode.hpp"
#include "MonitoringParameters.hpp"

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/MonitoredItem.h>

namespace magnesia::opcua_qt::abstraction {
    MonitoredItem::MonitoredItem(opcua::MonitoredItem<opcua::Client> item) : m_item(item) {}

    void MonitoredItem::setMonitoringMode(MonitoringMode mode) {
        m_item.setMonitoringMode(static_cast<opcua::MonitoringMode>(mode));
    }

    void MonitoredItem::setMonitoringParameters(MonitoringParameters parameters) {
        m_item.setMonitoringParameters(parameters.handle());
    }

    const opcua::MonitoredItem<opcua::Client>& MonitoredItem::handle() const noexcept {
        return m_item;
    }

    opcua::MonitoredItem<opcua::Client>& MonitoredItem::handle() noexcept {
        return m_item;
    }
} // namespace magnesia::opcua_qt::abstraction
