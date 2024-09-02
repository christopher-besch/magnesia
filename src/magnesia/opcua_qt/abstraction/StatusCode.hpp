#pragma once

#include <cstdint>

#include <open62541pp/types/Builtin.h>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class StatusCode
     * @brief A numerical value that is used to report the outcome of an operation performed by an OPC UA Server.
     *
     * See https://reference.opcfoundation.org/Core/Part4/v105/docs/7.39
     */
    class StatusCode {
      public:
        /**
         * @param status The StatusCode.
         */
        explicit StatusCode(opcua::StatusCode status);

        /**
         * Get the numeric value of the status.
         */
        [[nodiscard]] std::uint32_t get() const;

        /**
         * Get the name of the status code.
         */
        [[nodiscard]] QString toString() const;

        /**
         * Get the underlying status code.
         */
        [[nodiscard]] const opcua::StatusCode& handle() const noexcept;

        /**
         * Get the underlying status code.
         */
        [[nodiscard]] opcua::StatusCode& handle() noexcept;

      private:
        opcua::StatusCode m_status;
    };
} // namespace magnesia::opcua_qt::abstraction
