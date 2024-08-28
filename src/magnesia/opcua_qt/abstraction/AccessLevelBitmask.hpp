#pragma once

#include "AccessLevel.hpp"

#include <utility>
#include <vector>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * A bitmaks for AccessLevels.
     *
     * @see AccessLevel
     */
    class AccessLevelBitmask {
      public:
        explicit AccessLevelBitmask(opcua::Bitmask<opcua::AccessLevel> bitmask);

        /**
         * Get a list of all flags from AccessLevel and if they are set.
         *
         * @see AccessLevel
         *
         * See https://reference.opcfoundation.org/Core/Part3/v104/docs/8.57
         */
        [[nodiscard]] std::vector<std::pair<AccessLevel, bool>> getFlags() const;

        /**
         * Get a specific flag from AccessLevel.
         */
        [[nodiscard]] bool getFlag(AccessLevel flag) const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] const opcua::Bitmask<opcua::AccessLevel>& handle() const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] opcua::Bitmask<opcua::AccessLevel>& handle();

        /**
         * Set a specific flag from AccessLevel.
         */
        void setFlag(AccessLevel flag, bool set);

      private:
        [[nodiscard]] std::pair<AccessLevel, bool> getFlagPair(AccessLevel flag) const;

      private:
        opcua::Bitmask<opcua::AccessLevel> m_bitmask;
    };
} // namespace magnesia::opcua_qt::abstraction
