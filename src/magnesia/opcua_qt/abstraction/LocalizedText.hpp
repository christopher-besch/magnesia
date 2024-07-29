#pragma once

#include <open62541pp/types/Builtin.h>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * A text in a translation specified by the locale field.
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/8.5/
     */
    class LocalizedText {
      public:
        explicit LocalizedText(opcua::LocalizedText localized_text);
        LocalizedText(const QString& locale, const QString& text);

        /**
         * Get the locale. The format is the two letter ISO 639 code for a language followed by an optional hyphen and
         * the two letter ISO 3166 code for the country/region.
         */
        [[nodiscard]] QString getLocale() const noexcept;

        /**
         * Get the text.
         */
        [[nodiscard]] QString getText() const noexcept;

        /**
         * Get the underlying LocalizedText
         */
        [[nodiscard]] const opcua::LocalizedText& handle() const noexcept;

        /**
         * Get the underlying LocalizedText
         */
        [[nodiscard]] opcua::LocalizedText& handle() noexcept;

      private:
        opcua::LocalizedText m_text;
    };
} // namespace magnesia::opcua_qt::abstraction
