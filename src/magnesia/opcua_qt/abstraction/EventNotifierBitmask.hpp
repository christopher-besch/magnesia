#pragma once

#include "EventNotifier.hpp"

#include <utility>
#include <vector>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * A bitmaks for EventNotifiers.
     *
     * @see EventNotifier
     */
    class EventNotifierBitmask {
      public:
        explicit EventNotifierBitmask(opcua::Bitmask<opcua::EventNotifier> bitmask);

        /**
         * Get a list of all flags from EventNotifier and if they are set.
         *
         * @see EventNotifier
         *
         * See https://reference.opcfoundation.org/Core/Part3/v104/docs/8.57
         */
        [[nodiscard]] std::vector<std::pair<EventNotifier, bool>> getFlags() const;

        /**
         * Get a specific flag from EventNotifier.
         */
        [[nodiscard]] bool getFlag(EventNotifier flag) const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] const opcua::Bitmask<opcua::EventNotifier>& handle() const;

        /**
         * Get the underlying bitmask.
         */
        [[nodiscard]] opcua::Bitmask<opcua::EventNotifier>& handle();

        /**
         * Set a specific flag from EventNotifier.
         */
        void setFlag(EventNotifier flag, bool set);

      private:
        [[nodiscard]] std::pair<EventNotifier, bool> getFlagPair(EventNotifier flag) const;

      private:
        opcua::Bitmask<opcua::EventNotifier> m_bitmask;
    };
} // namespace magnesia::opcua_qt::abstraction
