#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class MonitoringMode
     * @brief The mode a MonitoredItem works under.
     *
     * @see MonitoredItem
     *
     * See https://reference.opcfoundation.org/Core/Part4/v105/docs/7.23
     */
    enum class MonitoringMode : int32_t {
        DISABLED,
        SAMPLING,
        REPORTING,
    };
} // namespace magnesia::opcua_qt::abstraction
