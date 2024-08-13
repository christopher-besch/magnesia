#include "ReferenceDescription.hpp"

#include "LocalizedText.hpp"
#include "NodeId.hpp"
#include "QualifiedName.hpp"

#include <utility>

#include <open62541pp/types/Composed.h>

namespace magnesia::opcua_qt::abstraction {
    ReferenceDescription::ReferenceDescription(opcua::ReferenceDescription description)
        : m_description(std::move(description)) {}

    NodeId ReferenceDescription::getReferenceType() const noexcept {
        return NodeId(m_description.getReferenceTypeId());
    }

    QualifiedName ReferenceDescription::getBrowseName() const noexcept {
        return QualifiedName(m_description.getBrowseName());
    }

    LocalizedText ReferenceDescription::getDisplayName() const noexcept {
        return LocalizedText(m_description.getDisplayName());
    }

    bool ReferenceDescription::isForward() const noexcept {
        return m_description.getIsForward();
    }

    const opcua::ReferenceDescription& ReferenceDescription::handle() const noexcept {
        return m_description;
    }

    opcua::ReferenceDescription& ReferenceDescription::handle() noexcept {
        return m_description;
    }
} // namespace magnesia::opcua_qt::abstraction
