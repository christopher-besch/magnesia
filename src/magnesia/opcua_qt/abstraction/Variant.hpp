#pragma once

#include "NodeId.hpp"

#include <optional>

#include <open62541pp/types/Variant.h>

#include <QtContainerFwd>

namespace magnesia::opcua_qt::abstraction {
    class Variant {
      public:
        explicit Variant(opcua::Variant variant);

        /**
         * Get if the value is a scalar (no array or struct). For example an int or a bool.
         */
        [[nodiscard]] bool isScalar() const noexcept;

        /**
         * Get if the value is an array.
         */
        [[nodiscard]] bool isArray() const noexcept;

        /**
         * Get a scalar value of type T.
         */
        template<typename T>
        [[nodiscard]] std::optional<T> getScalar() const;

        /**
         * Get an array value with elements of type T.
         */
        template<typename T>
        [[nodiscard]] std::optional<QList<T>> getArray() const;

        /**
         * Get a description of the data type.
         */
        [[nodiscard]] NodeId getDataType() const noexcept;

        /**
         * Convert to a QVariant. Will convert most types to Qt types.
         *
         * Types that can't be converted directly are represented as a QString.
         */
        [[nodiscard]] QVariant toQVariant() const;

        /**
         * Get a string representation of the value.
         */
        [[nodiscard]] QString toString() const noexcept;

        /**
         * Get the underlying variant.
         */
        [[nodiscard]] const opcua::Variant& handle() const noexcept;

        /**
         * Get the underlying variant.
         */
        [[nodiscard]] opcua::Variant& handle() noexcept;

      private:
        template<typename T>
        [[nodiscard]] QList<QVariant> getQVariantArray() const;

      private:
        opcua::Variant m_variant;
    };
} // namespace magnesia::opcua_qt::abstraction
