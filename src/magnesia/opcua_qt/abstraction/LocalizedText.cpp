#include "LocalizedText.hpp"

#include <string>
#include <utility>

#include <open62541pp/types/Builtin.h>

namespace magnesia::opcua_qt::abstraction {
    LocalizedText::LocalizedText(opcua::LocalizedText localized_text) : m_text(std::move(localized_text)) {}

    LocalizedText::LocalizedText(const QString& locale, const QString& text) {
        m_text = opcua::LocalizedText(locale.toStdString(), text.toStdString());
    }

    QString LocalizedText::getLocale() const noexcept {
        return QString::fromStdString(std::string(m_text.getLocale()));
    }

    QString LocalizedText::getText() const noexcept {
        return QString::fromStdString(std::string(m_text.getText()));
    }

    const opcua::LocalizedText& LocalizedText::handle() const noexcept {
        return m_text;
    }

    opcua::LocalizedText& LocalizedText::handle() noexcept {
        return m_text;
    }
} // namespace magnesia::opcua_qt::abstraction
