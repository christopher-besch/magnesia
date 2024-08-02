#pragma once

#include "StatusCode.hpp"
#include "Variant.hpp"

#include <cstdint>

#include <open62541pp/types/DataValue.h>

#include <QDateTime>
#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Contains a value as a Variant and metadata about it.
     *
     * @see Variant
     *
     * See https://reference.opcfoundation.org/Core/Part4/v104/docs/7.7
     */
    class DataValue {
      public:
        explicit DataValue(opcua::DataValue data_value);

        /**
         * Get the timestamp when the value was last updated.
         */
        [[nodiscard]] QDateTime getSourceTimestamp() const noexcept;

        /**
         * Get the picoseconds for the source timestamp.
         */
        [[nodiscard]] uint16_t getSourcePicoseconds() const noexcept;

        /**
         * Get the time when the server first knew of the new value.
         */
        [[nodiscard]] QDateTime getServerTimestamp() const noexcept;

        /**
         * Get the picoseconds for the server timestamp.
         */
        [[nodiscard]] uint16_t getServerPicoseconds() const noexcept;

        /**
         * Determines if the value can be used or if it's not good.
         */
        [[nodiscard]] StatusCode getStatusCode() const noexcept;

        /**
         * Get the value as a variant.
         *
         * @see Variant
         */
        [[nodiscard]] Variant getValue() const noexcept;

        /**
         * Get the name of the data type.
         */
        [[nodiscard]] QString getDataTypeName() const noexcept;

        /**
         * Set the value as a variant.
         *
         * @param value the new value as Variant
         */
        void setValue(Variant const& value);

        /**
         * Get the underlying opcua DataValue.
         */
        [[nodiscard]] const opcua::DataValue& handle() const noexcept;

        /**
         * Get the underlying opcua DataValue.
         */
        [[nodiscard]] opcua::DataValue& handle() noexcept;

      private:
        opcua::DataValue m_data_value;
    };
} // namespace magnesia::opcua_qt::abstraction
