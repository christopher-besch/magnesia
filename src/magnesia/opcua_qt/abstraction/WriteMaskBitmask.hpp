#pragma once

#include "WriteMask.hpp"

#include <utility>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

#include <QList>

namespace magnesia::opcua_qt::abstraction {
    /**
     * A bitmask for WriteMasks.
     *
     * @see WriteMask
     */
    class WriteMaskBitmask {
      public:
        explicit WriteMaskBitmask(opcua::Bitmask<opcua::WriteMask> bitmask);

        /**
         * Get a list of all flags from WriteMask and if they are set.
         *
         * @see WriteMask
         *
         * See https://reference.opcfoundation.org/Core/Part3/v104/docs/8.57
         */
        [[nodiscard]] QList<std::pair<WriteMask, bool>> getFlags() const;

        /**
         * Get a specific flag from WriteMask.
         */
        [[nodiscard]] bool getFlag(WriteMask flag) const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] const opcua::Bitmask<opcua::WriteMask>& handle() const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] opcua::Bitmask<opcua::WriteMask>& handle();

        /**
         * Set a specific flag from WriteMask.
         */
        void setFlag(WriteMask flag, bool set);

      private:
        [[nodiscard]] std::pair<WriteMask, bool> getFlagPair(WriteMask flag) const;

      private:
        opcua::Bitmask<opcua::WriteMask> m_bitmask;
    };
} // namespace magnesia::opcua_qt::abstraction
