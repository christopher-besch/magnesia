#include "StatusCode.hpp"

#include <cstdint>

#include <open62541pp/types/Builtin.h>

#include <QByteArrayView>

namespace magnesia::opcua_qt::abstraction {

    StatusCode::StatusCode(opcua::StatusCode status) : m_status(status) {}

    uint32_t StatusCode::get() const {
        return m_status.get();
    }

    QString StatusCode::toString() const {
        return QString::fromLatin1(QLatin1StringView{m_status.name()});
    }

    const opcua::StatusCode& StatusCode::handle() const noexcept {
        return m_status;
    }

    opcua::StatusCode& StatusCode::handle() noexcept {
        return m_status;
    }

} // namespace magnesia::opcua_qt::abstraction
