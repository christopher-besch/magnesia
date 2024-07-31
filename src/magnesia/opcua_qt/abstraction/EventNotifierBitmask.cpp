#include "EventNotifierBitmask.hpp"

#include "EventNotifier.hpp"

#include <utility>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

#include <QList>

namespace magnesia::opcua_qt::abstraction {
    EventNotifierBitmask::EventNotifierBitmask(opcua::Bitmask<opcua::EventNotifier> bitmask) : m_bitmask(bitmask) {}

    QList<std::pair<EventNotifier, bool>> EventNotifierBitmask::getFlags() {
        return {
            {getFlagPair(EventNotifier::SUBSCRIBE_TO_EVENTS)},
            {getFlagPair(EventNotifier::HISTORY_READ)},
            {getFlagPair(EventNotifier::HISTORY_WRITE)},
        };
    }

    std::pair<EventNotifier, bool> EventNotifierBitmask::getFlagPair(EventNotifier flag) {
        return {flag, getFlag(flag)};
    }

    bool EventNotifierBitmask::getFlag(EventNotifier flag) {
        return m_bitmask.anyOf(static_cast<opcua::EventNotifier>(flag));
    }

    const opcua::Bitmask<opcua::EventNotifier>& EventNotifierBitmask::handle() const {
        return m_bitmask;
    }

    opcua::Bitmask<opcua::EventNotifier>& EventNotifierBitmask::handle() {
        return m_bitmask;
    }

    void EventNotifierBitmask::setFlag(EventNotifier flag, bool set) {
        if (set) {
            m_bitmask.set(static_cast<opcua::EventNotifier>(flag));
        } else {
            m_bitmask.reset(static_cast<opcua::EventNotifier>(flag));
        }
    }
} // namespace magnesia::opcua_qt::abstraction
