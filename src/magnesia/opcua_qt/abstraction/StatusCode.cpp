#include "StatusCode.hpp"

#include <cstdint>

#include <open62541pp/types/Builtin.h>

namespace magnesia::opcua_qt::abstraction {

    StatusCode::StatusCode(opcua::StatusCode status) : m_status(status) {}

    uint32_t StatusCode::get() const {
        return m_status.get();
    }

    const opcua::StatusCode& StatusCode::handle() const noexcept {
        return m_status;
    }

    opcua::StatusCode& StatusCode::handle() noexcept {
        return m_status;
    }

} // namespace magnesia::opcua_qt::abstraction
