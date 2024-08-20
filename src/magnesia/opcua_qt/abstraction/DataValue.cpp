#include "DataValue.hpp"

#include "StatusCode.hpp"
#include "Variant.hpp"

#include <cstdint>
#include <utility>

#include <open62541pp/types/DataValue.h>

#include <QDateTime>
#include <QString>

namespace magnesia::opcua_qt::abstraction {
    DataValue::DataValue(opcua::DataValue data_value) : m_data_value(std::move(data_value)) {}

    QDateTime DataValue::getSourceTimestamp() const noexcept {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(m_data_value.getSourceTimestamp().toUnixTime());
        return timestamp;
    }

    uint16_t DataValue::getSourcePicoseconds() const noexcept {
        return m_data_value.getSourcePicoseconds();
    }

    QDateTime DataValue::getServerTimestamp() const noexcept {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(m_data_value.getServerTimestamp().toUnixTime());
        return timestamp;
    }

    uint16_t DataValue::getServerPicoseconds() const noexcept {
        return m_data_value.getSourcePicoseconds();
    }

    StatusCode DataValue::getStatusCode() const noexcept {
        return StatusCode(m_data_value.getStatus());
    }

    Variant DataValue::getValue() const noexcept {
        return Variant(m_data_value.getValue());
    }

    QString DataValue::getDataTypeName() const noexcept {
        return {m_data_value.getValue().getDataType()->typeName};
    }

    void DataValue::setValue(Variant const& value) {
        m_data_value.setValue(value.handle());
    }

    const opcua::DataValue& DataValue::handle() const noexcept {
        return m_data_value;
    }

    opcua::DataValue& DataValue::handle() noexcept {
        return m_data_value;
    }
} // namespace magnesia::opcua_qt::abstraction
