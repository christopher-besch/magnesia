#include "Variant.hpp"

#include "NodeId.hpp"
#include "qt_version_check.hpp"

#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <utility>

#include <open62541/types.h>
#include <open62541pp/types/Builtin.h>
#include <open62541pp/types/DateTime.h>
#include <open62541pp/types/NodeId.h>
#include <open62541pp/types/Variant.h>

#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>
#include <QtContainerFwd>

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
    std::optional<QList<T>> Variant::getArray() const {
        if (!m_variant.isArray()) {
            return std::nullopt;
        }

        auto vector = m_variant.getArray<T>();
        auto list   = QList<T>();

        list.reserve(static_cast<qsizetype>(vector.size()));
        std::copy(vector.begin(), vector.end(), std::back_inserter(list));

        return list;
    }

    template<typename T>
    QList<QVariant> Variant::getQVariantArray() const {
        auto vector = m_variant.getArray<T>();

        auto list = QList<QVariant>();
        list.reserve(static_cast<qsizetype>(vector.size()));

        for (const auto& var : vector) {
            list.append(QVariant::fromValue(var));
        }

        return list;
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
                    return {m_variant.getScalar<bool>()};
                case UA_DATATYPEKIND_SBYTE:
                    return {m_variant.getScalar<int8_t>()};
                case UA_DATATYPEKIND_BYTE:
                    return {m_variant.getScalar<uint8_t>()};
                case UA_DATATYPEKIND_INT16:
                    return {m_variant.getScalar<int16_t>()};
                case UA_DATATYPEKIND_UINT16:
                    return {m_variant.getScalar<uint16_t>()};
                case UA_DATATYPEKIND_INT32:
                    return {m_variant.getScalar<int32_t>()};
                case UA_DATATYPEKIND_UINT32:
                    return {m_variant.getScalar<uint32_t>()};
                case UA_DATATYPEKIND_INT64:
                    return QVariant::fromValue(m_variant.getScalar<int64_t>());
                case UA_DATATYPEKIND_UINT64:
                    return QVariant::fromValue(m_variant.getScalar<uint64_t>());
                case UA_DATATYPEKIND_FLOAT:
                    return QVariant::fromValue(m_variant.getScalar<float>());
                case UA_DATATYPEKIND_DOUBLE:
                    return QVariant::fromValue(m_variant.getScalar<double>());
                case UA_DATATYPEKIND_STRING:
                    return QVariant::fromValue(
                        QString::fromStdString(std::string(m_variant.getScalar<opcua::String>())));
                case UA_DATATYPEKIND_DATETIME:
                    return QVariant::fromValue(
                        QDateTime::fromSecsSinceEpoch(m_variant.getScalar<opcua::DateTime>().toUnixTime()));
                case UA_DATATYPEKIND_GUID:
                    return QVariant::fromValue(m_variant.getScalar<opcua::Guid>());
                case UA_DATATYPEKIND_STATUSCODE: {
                    return QVariant::fromValue(std::string(UA_StatusCode_name(m_variant.getScalar<UA_StatusCode>())));
                }
                default:
                    return QVariant::fromValue(
                        QString("<scalar unknown type kind: %1>").arg(m_variant.getDataType()->typeKind));
                    // TODO: Add data as hex
            }
        }

        if (isArray()) {
            switch (m_variant.getDataType()->typeKind) {
                case UA_DATATYPEKIND_BOOLEAN:
                    return {getQVariantArray<bool>()};
                case UA_DATATYPEKIND_SBYTE:
                    return {getQVariantArray<int8_t>()};
                case UA_DATATYPEKIND_BYTE:
                    return {getQVariantArray<uint8_t>()};
                case UA_DATATYPEKIND_INT16:
                    return {getQVariantArray<int16_t>()};
                case UA_DATATYPEKIND_UINT16:
                    return {getQVariantArray<uint16_t>()};
                case UA_DATATYPEKIND_INT32:
                    return {getQVariantArray<int32_t>()};
                case UA_DATATYPEKIND_UINT32:
                    return {getQVariantArray<uint32_t>()};
                case UA_DATATYPEKIND_INT64:
                    return {getQVariantArray<int64_t>()};
                case UA_DATATYPEKIND_UINT64:
                    return {getQVariantArray<uint64_t>()};
                case UA_DATATYPEKIND_FLOAT:
                    return {getQVariantArray<float>()};
                case UA_DATATYPEKIND_DOUBLE:
                    return {getQVariantArray<double>()};

                case UA_DATATYPEKIND_STRING: {
                    auto vector = m_variant.getArray<opcua::String>();

                    auto list = QList<QVariant>();
                    list.reserve(static_cast<qsizetype>(vector.size()));
                    for (const auto& var : vector) {
                        list.append(QVariant::fromValue(QString::fromStdString(std::string(var))));
                    }

                    return {list};
                }
                case UA_DATATYPEKIND_DATETIME: {
                    auto vector = m_variant.getArray<opcua::DateTime>();

                    auto list = QList<QVariant>();
                    list.reserve(static_cast<qsizetype>(vector.size()));
                    for (const auto& var : vector) {
                        list.append(QVariant::fromValue(QDateTime::fromSecsSinceEpoch(var.toUnixTime())));
                    }

                    return {list};
                }
                case UA_DATATYPEKIND_GUID:
                    return {getQVariantArray<opcua::Guid>()};
                case UA_DATATYPEKIND_STATUSCODE: {
                    auto vector = m_variant.getArray<UA_StatusCode>();

                    auto list = QList<QVariant>();
                    list.reserve(static_cast<qsizetype>(vector.size()));
                    for (const auto& var : vector) {
                        list.append(QVariant::fromValue(QString::fromStdString(std::string(UA_StatusCode_name(var)))));
                    }

                    return {list};
                }
                default:
                    return QVariant::fromValue(QString("<array(size: %1, type kind: %2)>")
                                                   .arg(m_variant.getArrayLength(), m_variant.getDataType()->typeKind));
            }
        }

        // TODO: Implement for complex types
        return {"<complex data type>"};
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
