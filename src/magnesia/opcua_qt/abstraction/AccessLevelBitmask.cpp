#include "AccessLevelBitmask.hpp"

#include "AccessLevel.hpp"

#include <utility>
#include <vector>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

namespace magnesia::opcua_qt::abstraction {
    AccessLevelBitmask::AccessLevelBitmask(opcua::Bitmask<opcua::AccessLevel> bitmask) : m_bitmask(bitmask) {}

    std::vector<std::pair<AccessLevel, bool>> AccessLevelBitmask::getFlags() const {
        return {
            getFlagPair(AccessLevel::CURRENT_READ),    getFlagPair(AccessLevel::CURRENT_WRITE),
            getFlagPair(AccessLevel::HISTORY_READ),    getFlagPair(AccessLevel::HISTORY_WRITE),
            getFlagPair(AccessLevel::SEMANTIC_CHANGE), getFlagPair(AccessLevel::STATUS_WRITE),
            getFlagPair(AccessLevel::TIMESTAMP_WRITE),
        };
    }

    std::pair<AccessLevel, bool> AccessLevelBitmask::getFlagPair(AccessLevel flag) const {
        return {flag, getFlag(flag)};
    }

    bool AccessLevelBitmask::getFlag(AccessLevel flag) const {
        return m_bitmask.anyOf(static_cast<opcua::AccessLevel>(flag));
    }

    const opcua::Bitmask<opcua::AccessLevel>& AccessLevelBitmask::handle() const {
        return m_bitmask;
    }

    opcua::Bitmask<opcua::AccessLevel>& AccessLevelBitmask::handle() {
        return m_bitmask;
    }

    void AccessLevelBitmask::setFlag(AccessLevel flag, bool set) {
        if (set) {
            m_bitmask.set(static_cast<opcua::AccessLevel>(flag));
        } else {
            m_bitmask.reset(static_cast<opcua::AccessLevel>(flag));
        }
    }
} // namespace magnesia::opcua_qt::abstraction
