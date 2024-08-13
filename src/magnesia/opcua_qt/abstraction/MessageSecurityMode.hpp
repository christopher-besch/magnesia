#pragma once

#include <cstdint>

namespace magnesia::opcua_qt {
    /**
     * The security mode of a connection.
     *
     * @see Connection
     */
    enum class MessageSecurityMode : uint32_t {
        INVALID,
        NONE,
        SIGN,
        SIGN_AND_ENCRYPT,
    };
} // namespace magnesia::opcua_qt
