#include "QualifiedName.hpp"

#include <cstdint>
#include <string>
#include <utility>

#include <open62541pp/types/Builtin.h>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    QualifiedName::QualifiedName(opcua::QualifiedName qualified_name) : m_qualified_name(std::move(qualified_name)) {}

    QualifiedName::QualifiedName(uint16_t namespace_index, QString& name) {
        m_qualified_name = opcua::QualifiedName(namespace_index, name.toStdString());
    }

    uint16_t QualifiedName::getNameSpaceIndex() const {
        return m_qualified_name.getNamespaceIndex();
    }

    QString QualifiedName::getName() const {
        return QString::fromStdString(std::string(m_qualified_name.getName()));
    }

    const opcua::QualifiedName& QualifiedName::handle() const noexcept {
        return m_qualified_name;
    }

    opcua::QualifiedName& QualifiedName::handle() noexcept {
        return m_qualified_name;
    }
} // namespace magnesia::opcua_qt::abstraction
