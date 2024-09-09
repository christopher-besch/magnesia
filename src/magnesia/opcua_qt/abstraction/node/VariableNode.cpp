#include "VariableNode.hpp"

#include "../../../qt_version_check.hpp"
#include "../AccessLevelBitmask.hpp"
#include "../DataValue.hpp"
#include "../NodeId.hpp"
#include "../ValueRank.hpp"
#include "Node.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Node.h>

#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    VariableNode::VariableNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {}

    const DataValue* VariableNode::getDataValue() {
        try {
            return &wrapCache(&Cache::data_value, [this] { return DataValue{handle().readDataValue()}; });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<NodeId> VariableNode::getDataType() {
        try {
            return wrapCache(&Cache::data_type, [this] { return NodeId{handle().readDataType()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<ValueRank> VariableNode::getValueRank() {
        try {
            return wrapCache(&Cache::value_rank, [this] { return static_cast<ValueRank>(handle().readValueRank()); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    const std::vector<std::uint32_t>* VariableNode::getArrayDimensions() {
        try {
            return &wrapCache(&Cache::array_dimensions, [this] { return handle().readArrayDimensions(); });
        } catch (const opcua::BadStatus&) {
            return nullptr;
        }
    }

    std::optional<AccessLevelBitmask> VariableNode::getAccessLevel() {
        try {
            return wrapCache(&Cache::access_level, [this] { return AccessLevelBitmask{handle().readAccessLevel()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<AccessLevelBitmask> VariableNode::getUserAccessLevel() {
        try {
            return wrapCache(&Cache::user_access_level,
                             [this] { return AccessLevelBitmask{handle().readUserAccessLevel()}; });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<double> VariableNode::getMinimumSamplingInterval() {
        try {
            return wrapCache(&Cache::minimum_sampling_interval,
                             [this] { return handle().readMinimumSamplingInterval(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }

    std::optional<bool> VariableNode::isHistorizing() {
        try {
            return wrapCache(&Cache::is_historizing, [this] { return handle().readHistorizing(); });
        } catch (const opcua::BadStatus&) {
            return std::nullopt;
        }
    }
} // namespace magnesia::opcua_qt::abstraction
