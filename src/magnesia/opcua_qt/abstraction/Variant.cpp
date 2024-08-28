#include "Variant.hpp"

#include "../../qt_version_check.hpp"
#include "NodeId.hpp"

#include <cstdint>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include <open62541/types.h>
#include <open62541pp/types/Builtin.h>
#include <open62541pp/types/DateTime.h>
#include <open62541pp/types/NodeId.h>
#include <open62541pp/types/Variant.h>

#include <QByteArrayView>
#include <QDateTime>
#include <QString>
#include <QVariant>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    Variant::Variant(opcua::Variant variant) : m_variant(std::move(variant)) {}

    bool Variant::isScalar() const noexcept {
        return m_variant.isScalar();
    }

    bool Variant::isArray() const noexcept {
        return m_variant.isArray();
    }

    template<typename T>
    std::optional<T> Variant::getScalar() const {
        if (!m_variant.isScalar()) {
            return std::nullopt;
        }

        return m_variant.getScalar<T>();
    }

    template<typename T>
    std::optional<std::vector<T>> Variant::getArray() const {
        if (!m_variant.isArray()) {
            return std::nullopt;
        }

        auto span = m_variant.getArray<T>();
        return {span.begin(), span.end()};
    }

    template<typename T>
    std::vector<QVariant> Variant::getQVariantArray() const {
        auto span = m_variant.getArray<T>();
        auto res  = std::views::transform(span, [](const auto& val) { return QVariant::fromValue(val); });
        return {res.begin(), res.end()};
    }

    NodeId Variant::getDataType() const noexcept {
        return NodeId(opcua::NodeId(m_variant.getDataType()->typeId));
    }

    QVariant Variant::toQVariant() const {
        if (m_variant.isEmpty()) {
            return {};
        }

        if (isScalar()) {
            switch (m_variant.getDataType()->typeKind) {
                case UA_DATATYPEKIND_BOOLEAN:
                    return m_variant.getScalar<bool>();
                case UA_DATATYPEKIND_SBYTE:
                    return m_variant.getScalar<int8_t>();
                case UA_DATATYPEKIND_BYTE:
                    return m_variant.getScalar<uint8_t>();
                case UA_DATATYPEKIND_INT16:
                    return m_variant.getScalar<int16_t>();
                case UA_DATATYPEKIND_UINT16:
                    return m_variant.getScalar<uint16_t>();
                case UA_DATATYPEKIND_INT32:
                    return m_variant.getScalar<int32_t>();
                case UA_DATATYPEKIND_UINT32:
                    return m_variant.getScalar<uint32_t>();
                case UA_DATATYPEKIND_INT64:
                    return QVariant::fromValue(m_variant.getScalar<int64_t>());
                case UA_DATATYPEKIND_UINT64:
                    return QVariant::fromValue(m_variant.getScalar<uint64_t>());
                case UA_DATATYPEKIND_FLOAT:
                    return QVariant::fromValue(m_variant.getScalar<float>());
                case UA_DATATYPEKIND_DOUBLE:
                    return QVariant::fromValue(m_variant.getScalar<double>());
                case UA_DATATYPEKIND_STRING:
                    return QString{QLatin1StringView{m_variant.getScalar<opcua::String>().get()}};
                case UA_DATATYPEKIND_DATETIME:
                    return QDateTime::fromSecsSinceEpoch(m_variant.getScalar<opcua::DateTime>().toUnixTime());
                case UA_DATATYPEKIND_GUID:
                    return QVariant::fromValue(m_variant.getScalar<opcua::Guid>());
                case UA_DATATYPEKIND_STATUSCODE: {
                    return UA_StatusCode_name(m_variant.getScalar<UA_StatusCode>());
                }
                default:
                    return QString{"<scalar unknown type kind: %1>"}.arg(m_variant.getDataType()->typeKind);
                    // TODO: Add data as hex
            }
        }

        if (isArray()) {
            switch (m_variant.getDataType()->typeKind) {
                case UA_DATATYPEKIND_BOOLEAN:
                    return QVariant::fromValue(getQVariantArray<bool>());
                case UA_DATATYPEKIND_SBYTE:
                    return QVariant::fromValue(getQVariantArray<int8_t>());
                case UA_DATATYPEKIND_BYTE:
                    return QVariant::fromValue(getQVariantArray<uint8_t>());
                case UA_DATATYPEKIND_INT16:
                    return QVariant::fromValue(getQVariantArray<int16_t>());
                case UA_DATATYPEKIND_UINT16:
                    return QVariant::fromValue(getQVariantArray<uint16_t>());
                case UA_DATATYPEKIND_INT32:
                    return QVariant::fromValue(getQVariantArray<int32_t>());
                case UA_DATATYPEKIND_UINT32:
                    return QVariant::fromValue(getQVariantArray<uint32_t>());
                case UA_DATATYPEKIND_INT64:
                    return QVariant::fromValue(getQVariantArray<int64_t>());
                case UA_DATATYPEKIND_UINT64:
                    return QVariant::fromValue(getQVariantArray<uint64_t>());
                case UA_DATATYPEKIND_FLOAT:
                    return QVariant::fromValue(getQVariantArray<float>());
                case UA_DATATYPEKIND_DOUBLE:
                    return QVariant::fromValue(getQVariantArray<double>());

                case UA_DATATYPEKIND_STRING: {
                    auto span = m_variant.getArray<opcua::String>();
                    auto res  = std::views::transform(
                        span, [](const opcua::String& string) { return QLatin1StringView{string.get()}; });
                    return QVariant::fromValue(std::vector<QVariant>{res.begin(), res.end()});
                }
                case UA_DATATYPEKIND_DATETIME: {
                    auto span = m_variant.getArray<opcua::DateTime>();
                    auto res  = std::views::transform(span, [](const opcua::DateTime& val) {
                        return QDateTime::fromSecsSinceEpoch(val.toUnixTime());
                    });
                    return QVariant::fromValue(std::vector<QVariant>{res.begin(), res.end()});
                }
                case UA_DATATYPEKIND_GUID:
                    return QVariant::fromValue(getQVariantArray<opcua::Guid>());
                case UA_DATATYPEKIND_STATUSCODE: {
                    auto span = m_variant.getArray<UA_StatusCode>();
                    auto res  = std::views::transform(span, UA_StatusCode_name);
                    return QVariant::fromValue(std::vector<QVariant>{res.begin(), res.end()});
                }
                default:
                    return QString{"<array(size: %1, type kind: %2)>"}.arg(m_variant.getArrayLength(),
                                                                           m_variant.getDataType()->typeKind);
            }
        }

        // TODO: Implement for complex types
        return "<complex data type>";
    }

    QString Variant::toString() const noexcept {
        return toQVariant().toString();
    }

    const opcua::Variant& Variant::handle() const noexcept {
        return m_variant;
    }

    opcua::Variant& Variant::handle() noexcept {
        return m_variant;
    }
} // namespace magnesia::opcua_qt::abstraction
