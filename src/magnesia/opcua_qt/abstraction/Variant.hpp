#pragma once

#include "AccessLevelBitmask.hpp"
#include "EventNotifierBitmask.hpp"
#include "LocalizedText.hpp"
#include "NodeClass.hpp"
#include "NodeId.hpp"
#include "QualifiedName.hpp"
#include "WriteMaskBitmask.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"

#include <cstdint>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>
#include <open62541pp/types/Builtin.h>
#include <open62541pp/types/NodeId.h>
#include <open62541pp/types/Variant.h>

#include <QString>
#include <QVariant>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class Variant
     * @brief Union of different OPC UA types.
     *
     * See https://open62541pp.github.io/open62541pp/classopcua_1_1Variant.html
     */
    class Variant {
      public:
        /**
         * @param variant OPC UA variant.
         */
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
        [[nodiscard]] std::optional<T> getScalar() const {
            if (!m_variant.isScalar()) {
                return std::nullopt;
            }

            return m_variant.getScalar<T>();
        }

        /**
         * Get an array value with elements of type T.
         */
        template<typename T>
        [[nodiscard]] std::optional<std::vector<T>> getArray() const {
            if (!m_variant.isArray()) {
                return std::nullopt;
            }

            auto span = m_variant.getArray<T>();
            return std::optional<std::vector<T>>{std::in_place, span.begin(), span.end()};
        }

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
        [[nodiscard]] std::vector<QVariant> getQVariantArray() const {
            auto span = m_variant.getArray<T>();
            auto res  = std::views::transform(span, [](const auto& val) { return QVariant::fromValue(val); });
            return {res.begin(), res.end()};
        }

        template<typename From, typename To>
        [[nodiscard]] std::optional<To> getWrapper() const {
            if (!m_variant.isScalar()) {
                return std::nullopt;
            }

            return static_cast<To>(m_variant.getScalar<From>());
        }

        template<typename From, typename To>
        [[nodiscard]] std::optional<To> getBitmask() const {
            if (!m_variant.isScalar()) {
                return std::nullopt;
            }

            return static_cast<To>(opcua::Bitmask<From>{m_variant.getScalar<std::underlying_type_t<From>>()});
        }

      private:
        opcua::Variant m_variant;
    };

    template<>
    [[nodiscard]] inline std::optional<NodeId> Variant::getScalar() const {
        return getWrapper<opcua::NodeId, NodeId>();
    }

    template<>
    [[nodiscard]] inline std::optional<LocalizedText> Variant::getScalar() const {
        return getWrapper<opcua::LocalizedText, LocalizedText>();
    }

    template<>
    [[nodiscard]] inline std::optional<QualifiedName> Variant::getScalar() const {
        return getWrapper<opcua::QualifiedName, QualifiedName>();
    }

    template<>
    [[nodiscard]] inline std::optional<WriteMaskBitmask> Variant::getScalar() const {
        return getBitmask<opcua::WriteMask, WriteMaskBitmask>();
    }

    template<>
    [[nodiscard]] inline std::optional<EventNotifierBitmask> Variant::getScalar() const {
        return getBitmask<opcua::EventNotifier, EventNotifierBitmask>();
    }

    template<>
    [[nodiscard]] inline std::optional<AccessLevelBitmask> Variant::getScalar() const {
        return getBitmask<opcua::AccessLevel, AccessLevelBitmask>();
    }

    template<>
    [[nodiscard]] inline std::optional<NodeClass> Variant::getScalar() const {
        return getWrapper<std::int32_t, NodeClass>();
    }

    template<>
    [[nodiscard]] inline std::optional<ValueRank> Variant::getScalar() const {
        return getWrapper<std::int32_t, ValueRank>();
    }
} // namespace magnesia::opcua_qt::abstraction
