#include "WriteMaskBitmask.hpp"

#include "WriteMask.hpp"

#include <utility>

#include <open62541pp/Bitmask.h>
#include <open62541pp/Common.h>

#include <QDateTime>
#include <QList>
#include <QtContainerFwd>

namespace magnesia::opcua_qt::abstraction {
    WriteMaskBitmask::WriteMaskBitmask(opcua::Bitmask<opcua::WriteMask> bitmask) : m_bitmask(bitmask) {}

    QList<std::pair<WriteMask, bool>> WriteMaskBitmask::getFlags() {
        return {
            {getFlagPair(WriteMask::ACCESS_LEVEL)},
            {getFlagPair(WriteMask::ARRAY_DIMENSIONS)},
            {getFlagPair(WriteMask::BROWSE_NAME)},
            {getFlagPair(WriteMask::CONTAINS_NO_LOOPS)},
            {getFlagPair(WriteMask::DATA_TYPE)},
            {getFlagPair(WriteMask::DESCRIPTION)},
            {getFlagPair(WriteMask::DISPLAY_NAME)},
            {getFlagPair(WriteMask::EVENT_NOTIFIER)},
            {getFlagPair(WriteMask::EXECUTABLE)},
            {getFlagPair(WriteMask::HISTORIZING)},
            {getFlagPair(WriteMask::INVERSE_NAME)},
            {getFlagPair(WriteMask::IS_ABSTRACT)},
            {getFlagPair(WriteMask::MINIMUM_SAMPLING_INTERVAL)},
            {getFlagPair(WriteMask::NODE_CLASS)},
            {getFlagPair(WriteMask::NODE_ID)},
            {getFlagPair(WriteMask::SYMMETRIC)},
            {getFlagPair(WriteMask::USER_ACCESS_LEVEL)},
            {getFlagPair(WriteMask::USER_EXECUTABLE)},
            {getFlagPair(WriteMask::USER_WRITE_MASK)},
            {getFlagPair(WriteMask::VALUE_RANK)},
            {getFlagPair(WriteMask::WRITE_MASK)},
            {getFlagPair(WriteMask::VALUE_FOR_VARIABLE_TYPE)},
            {getFlagPair(WriteMask::DATA_TYPE_DEFINITION)},
            {getFlagPair(WriteMask::ROLE_PERMISSIONS)},
            {getFlagPair(WriteMask::ACCESS_RESTRICTIONS)},
            {getFlagPair(WriteMask::ACCESS_LEVEL_EX)},
        };
    }

    std::pair<WriteMask, bool> WriteMaskBitmask::getFlagPair(WriteMask flag) {
        return {flag, getFlag(flag)};
    }

    bool WriteMaskBitmask::getFlag(WriteMask flag) {
        return m_bitmask.anyOf(static_cast<opcua::WriteMask>(flag));
    }

    const opcua::Bitmask<opcua::WriteMask>& WriteMaskBitmask::handle() const {
        return m_bitmask;
    }

    opcua::Bitmask<opcua::WriteMask>& WriteMaskBitmask::handle() {
        return m_bitmask;
    }

    void WriteMaskBitmask::setFlag(WriteMask flag, bool set) {
        if (set) {
            m_bitmask.set(static_cast<opcua::WriteMask>(flag));
        } else {
            m_bitmask.reset(static_cast<opcua::WriteMask>(flag));
        }
    }
} // namespace magnesia::opcua_qt::abstraction
