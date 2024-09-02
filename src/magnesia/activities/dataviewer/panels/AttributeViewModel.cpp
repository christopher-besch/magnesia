#include "AttributeViewModel.hpp"

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/AccessLevel.hpp"
#include "../../../opcua_qt/abstraction/AttributeId.hpp"
#include "../../../opcua_qt/abstraction/EventNotifier.hpp"
#include "../../../opcua_qt/abstraction/NodeClass.hpp"
#include "../../../opcua_qt/abstraction/ValueRank.hpp"
#include "../../../opcua_qt/abstraction/WriteMask.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtContainerFwd>

using magnesia::opcua_qt::Connection;
using magnesia::opcua_qt::abstraction::access_level_to_string;
using magnesia::opcua_qt::abstraction::AttributeId;
using magnesia::opcua_qt::abstraction::event_notifier_to_string;
using magnesia::opcua_qt::abstraction::Node;
using magnesia::opcua_qt::abstraction::node_class_to_string;
using magnesia::opcua_qt::abstraction::value_rank_to_string;
using magnesia::opcua_qt::abstraction::write_mask_to_string;

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {

    AttributeViewModel::AttributeViewModel(QObject* parent) : QAbstractItemModel(parent), m_node() {}

    void AttributeViewModel::setNode(Node* node, Connection* connection) {
        beginResetModel();

        std::optional<Node*> type_node = std::nullopt;
        auto                 type_id   = node->getDataType();

        if (type_id.has_value()) {
            type_node = connection->getNode(type_id.value());
        }

        m_node = {
            .node_id                   = node->getNodeId(),
            .node_class                = node->getNodeClass(),
            .browse_name               = node->getBrowseName(),
            .display_name              = node->getDisplayName(),
            .description               = node->getDescription(),
            .write_mask                = node->getWriteMask(),
            .user_write_mask           = node->getUserWriteMask(),
            .is_abstract               = node->isAbstract(),
            .is_symmetric              = node->isSymmetric(),
            .inverse_name              = node->getInverseName(),
            .contains_no_loops         = node->containsNoLoops(),
            .event_notifier            = node->getEventNotifierType(),
            .data_value                = node->getDataValue(),
            .data_type                 = type_node,
            .value_rank                = node->getValueRank(),
            .array_dimensions          = node->getArrayDimensions(),
            .access_level              = node->getAccessLevel(),
            .user_access_level         = node->getUserAccessLevel(),
            .minimum_sampling_interval = node->getMinimumSamplingInterval(),
            .is_historizing            = node->isHistorizing(),
            .is_executable             = node->isExecutable(),
            .is_user_executable        = node->isUserExecutable(),
        };

        m_available_attributes.clear();

        if (m_node.node_id) {
            m_available_attributes.push_back(AttributeId::NODE_ID);
        }

        if (m_node.node_class) {
            m_available_attributes.push_back(AttributeId::NODE_CLASS);
        }

        if (m_node.browse_name) {
            m_available_attributes.push_back(AttributeId::BROWSE_NAME);
        }

        if (m_node.display_name) {
            m_available_attributes.push_back(AttributeId::DISPLAY_NAME);
        }

        if (m_node.description) {
            m_available_attributes.push_back(AttributeId::DESCRIPTION);
        }

        if (m_node.write_mask) {
            m_available_attributes.push_back(AttributeId::WRITE_MASK);
        }

        if (m_node.user_write_mask) {
            m_available_attributes.push_back(AttributeId::USER_WRITE_MASK);
        }

        if (m_node.is_abstract) {
            m_available_attributes.push_back(AttributeId::IS_ABSTRACT);
        }

        if (m_node.is_symmetric) {
            m_available_attributes.push_back(AttributeId::SYMMETRIC);
        }

        if (m_node.inverse_name) {
            m_available_attributes.push_back(AttributeId::INVERSE_NAME);
        }

        if (m_node.contains_no_loops) {
            m_available_attributes.push_back(AttributeId::CONTAINS_NO_LOOPS);
        }

        if (m_node.event_notifier) {
            m_available_attributes.push_back(AttributeId::EVENT_NOTFIER);
        }

        if (m_node.data_value) {
            m_available_attributes.push_back(AttributeId::VALUE);
        }

        if (m_node.data_type) {
            m_available_attributes.push_back(AttributeId::DATA_TYPE);
        }

        if (m_node.value_rank) {
            m_available_attributes.push_back(AttributeId::VALUE_RANK);
        }

        if (m_node.array_dimensions) {
            m_available_attributes.push_back(AttributeId::ARRAY_DIMENSIONS);
        }

        if (m_node.access_level) {
            m_available_attributes.push_back(AttributeId::ACCESS_LEVEL);
        }

        if (m_node.user_access_level) {
            m_available_attributes.push_back(AttributeId::USER_ACCESS_LEVEL);
        }

        if (m_node.minimum_sampling_interval) {
            m_available_attributes.push_back(AttributeId::MINIMUM_SAMPLING_INTERVAL);
        }

        if (m_node.is_historizing) {
            m_available_attributes.push_back(AttributeId::HISTORIZING);
        }

        if (m_node.is_executable) {
            m_available_attributes.push_back(AttributeId::EXECUTABLE);
        }

        if (m_node.is_user_executable) {
            m_available_attributes.push_back(AttributeId::USER_EXECUTABLE);
        }

        endResetModel();
    }

    QModelIndex AttributeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (row < 1) {
            return {};
        }

        std::uint32_t item_id = 0;

        if (!parent.isValid()) {
            item_id = itemId(m_available_attributes[static_cast<std::size_t>(row - 1)], 0);
        } else {
            auto sub_item = static_cast<std::uint8_t>(row);
            item_id       = itemId(m_available_attributes[static_cast<std::size_t>(parent.row() - 1)], sub_item);
        }

        return createIndex(row, column, item_id);
    }

    QModelIndex AttributeViewModel::parent(const QModelIndex& index) const {
        if (!index.isValid()) {
            return {};
        }

        auto item_id  = itemId(index);
        auto sub_item = subId(item_id);
        if (sub_item == 0) {
            return {};
        }

        auto attribute = attributeId(item_id);
        auto row =
            std::distance(m_available_attributes.begin(), std::ranges::find(m_available_attributes, attribute)) + 1;

        return createIndex(static_cast<int>(row), index.column(), static_cast<std::uint32_t>(attribute));
    }

    int AttributeViewModel::rowCount(const QModelIndex& parent) const {
        if (!parent.isValid()) {
            return static_cast<int>(m_available_attributes.size() + 1);
        }

        auto item_id      = itemId(parent);
        auto sub_item     = subId(item_id);
        auto attribute_id = attributeId(item_id);

        if (sub_item != 0 || std::ranges::find(m_available_attributes, attribute_id) == m_available_attributes.end()) {
            return 0;
        }

        switch (attributeId(item_id)) {
            case opcua_qt::abstraction::AttributeId::NODE_CLASS:
            case opcua_qt::abstraction::AttributeId::IS_ABSTRACT:
            case opcua_qt::abstraction::AttributeId::SYMMETRIC:
            case opcua_qt::abstraction::AttributeId::CONTAINS_NO_LOOPS:
            case opcua_qt::abstraction::AttributeId::VALUE_RANK:
            case opcua_qt::abstraction::AttributeId::MINIMUM_SAMPLING_INTERVAL:
            case opcua_qt::abstraction::AttributeId::HISTORIZING:
            case opcua_qt::abstraction::AttributeId::EXECUTABLE:
            case opcua_qt::abstraction::AttributeId::USER_EXECUTABLE:
                return 0;

            case opcua_qt::abstraction::AttributeId::NODE_ID:
            case opcua_qt::abstraction::AttributeId::BROWSE_NAME:
            case opcua_qt::abstraction::AttributeId::DISPLAY_NAME:
            case opcua_qt::abstraction::AttributeId::DESCRIPTION:
            case opcua_qt::abstraction::AttributeId::INVERSE_NAME:
                return 3;

            case opcua_qt::abstraction::AttributeId::EVENT_NOTFIER:
                return 4;

            case opcua_qt::abstraction::AttributeId::ACCESS_LEVEL:
            case opcua_qt::abstraction::AttributeId::USER_ACCESS_LEVEL:
                return 8; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            case opcua_qt::abstraction::AttributeId::WRITE_MASK:
            case opcua_qt::abstraction::AttributeId::USER_WRITE_MASK:
                return 26; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            case opcua_qt::abstraction::AttributeId::ARRAY_DIMENSIONS:
                if (!m_node.array_dimensions.has_value()) {
                    return 0;
                }

                return static_cast<int>(m_node.array_dimensions->size() + 1);

            case opcua_qt::abstraction::AttributeId::ACCESS_RESTRICTIONS:
            case opcua_qt::abstraction::AttributeId::ACCESS_LEVEL_EX:
            case opcua_qt::abstraction::AttributeId::VALUE:
            case opcua_qt::abstraction::AttributeId::DATA_TYPE:
            case opcua_qt::abstraction::AttributeId::DATA_TYPE_DEFINITION:
            case opcua_qt::abstraction::AttributeId::ROLE_PERMISSIONS:
            case opcua_qt::abstraction::AttributeId::USER_ROLE_PERMISSIONS:
                return 0; // TODO: Implement
                break;
        }

        return 0;
    }

    int AttributeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 2;
    }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity)
    QVariant AttributeViewModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return {};
        }

        const std::uint32_t item_id      = itemId(index);
        const AttributeId   attribute_id = attributeId(item_id);
        const std::uint8_t  sub_item     = subId(item_id);
        const bool          title        = index.column() == 0;

        if (item_id == itemId(AttributeId::NODE_ID, 0) && m_node.node_id.has_value()) {
            return title ? "Node Id" : m_node.node_id.value().toString();
        }
        if (item_id == itemId(AttributeId::NODE_ID, 1) && m_node.node_id.has_value()) {
            return title ? "Namespace Index" : QVariant::fromValue(m_node.node_id.value().getNamespaceIndex());
        }
        if (item_id == itemId(AttributeId::NODE_ID, 2) && m_node.node_id.has_value()) {
            return title ? "Identifier" : m_node.node_id->getIdentifier();
        }

        if (item_id == itemId(AttributeId::NODE_CLASS, 0) && m_node.node_class.has_value()) {
            return title ? "Node Class" : node_class_to_string(m_node.node_class.value());
        }

        if (item_id == itemId(AttributeId::BROWSE_NAME, 0) && m_node.browse_name.has_value()) {
            return title ? "Browse Name" : m_node.browse_name->getName();
        }
        if (item_id == itemId(AttributeId::BROWSE_NAME, 1) && m_node.browse_name.has_value()) {
            return title ? "Namespace Index" : QVariant::fromValue(m_node.browse_name->getNameSpaceIndex());
        }
        if (item_id == itemId(AttributeId::BROWSE_NAME, 2) && m_node.browse_name.has_value()) {
            return title ? "Name" : m_node.browse_name->getName();
        }

        if (item_id == itemId(AttributeId::DISPLAY_NAME, 0) && m_node.display_name.has_value()) {
            return title ? "Display Name" : m_node.display_name->getText();
        }
        if (item_id == itemId(AttributeId::DISPLAY_NAME, 1) && m_node.display_name.has_value()) {
            return title ? "Text" : m_node.display_name->getText();
        }
        if (item_id == itemId(AttributeId::DISPLAY_NAME, 2) && m_node.display_name.has_value()) {
            return title ? "Locale" : m_node.display_name->getLocale();
        }

        if (item_id == itemId(AttributeId::DESCRIPTION, 0) && m_node.description.has_value()) {
            return title ? "Description" : m_node.description->getText();
        }
        if (item_id == itemId(AttributeId::DESCRIPTION, 1) && m_node.description.has_value()) {
            return title ? "Text" : m_node.description->getText();
        }
        if (item_id == itemId(AttributeId::DESCRIPTION, 2) && m_node.description.has_value()) {
            return title ? "Locale" : m_node.description->getLocale();
        }

        if (item_id == itemId(AttributeId::INVERSE_NAME, 0) && m_node.inverse_name.has_value()) {
            return title ? "Inverse Name" : m_node.inverse_name->getText();
        }
        if (item_id == itemId(AttributeId::INVERSE_NAME, 1) && m_node.inverse_name.has_value()) {
            return title ? "Text" : m_node.inverse_name->getText();
        }
        if (item_id == itemId(AttributeId::INVERSE_NAME, 2) && m_node.inverse_name.has_value()) {
            return title ? "Locale" : m_node.inverse_name->getLocale();
        }

        if (attribute_id == AttributeId::WRITE_MASK && m_node.write_mask.has_value()) {
            if (sub_item == 0) {
                return title ? "Write Mask" : QVariant();
            }

            auto pair = m_node.write_mask->getFlags()[sub_item - 1];
            return title ? write_mask_to_string(pair.first) : QVariant::fromValue(pair.second);
        }
        if (attribute_id == AttributeId::USER_WRITE_MASK && m_node.user_write_mask.has_value()) {
            if (sub_item == 0) {
                return title ? "User Write Mask" : QVariant();
            }

            auto pair = m_node.user_write_mask->getFlags()[sub_item - 1];
            return title ? write_mask_to_string(pair.first) : QVariant::fromValue(pair.second);
        }
        if (attribute_id == AttributeId::ACCESS_LEVEL && m_node.access_level.has_value()) {
            if (sub_item == 0) {
                return title ? "Access Level" : QVariant();
            }

            auto pair = m_node.access_level->getFlags()[sub_item - 1];
            return title ? access_level_to_string(pair.first) : QVariant::fromValue(pair.second);
        }
        if (attribute_id == AttributeId::USER_ACCESS_LEVEL && m_node.user_access_level.has_value()) {
            if (sub_item == 0) {
                return title ? "User Access Level" : QVariant();
            }

            auto pair = m_node.user_access_level->getFlags()[sub_item - 1];
            return title ? access_level_to_string(pair.first) : QVariant::fromValue(pair.second);
        }
        if (attribute_id == AttributeId::EVENT_NOTFIER && m_node.event_notifier.has_value()) {
            if (sub_item == 0) {
                return title ? "Event Notifier" : QVariant();
            }

            auto pair = m_node.event_notifier->getFlags()[sub_item - 1];
            return title ? event_notifier_to_string(pair.first) : QVariant::fromValue(pair.second);
        }

        if (attribute_id == AttributeId::IS_ABSTRACT && m_node.is_abstract.has_value()) {
            return title ? "Is Abstract" : QVariant::fromValue(m_node.is_abstract.value());
        }
        if (attribute_id == AttributeId::SYMMETRIC && m_node.is_symmetric.has_value()) {
            return title ? "Is Symmetric" : QVariant::fromValue(m_node.is_symmetric.value());
        }
        if (attribute_id == AttributeId::CONTAINS_NO_LOOPS && m_node.contains_no_loops.has_value()) {
            return title ? "Contains no loops" : QVariant::fromValue(m_node.contains_no_loops.value());
        }
        if (attribute_id == AttributeId::MINIMUM_SAMPLING_INTERVAL && m_node.minimum_sampling_interval.has_value()) {
            return title ? "Minimum Sampling Interval" : QVariant::fromValue(m_node.minimum_sampling_interval.value());
        }
        if (attribute_id == AttributeId::HISTORIZING && m_node.is_historizing.has_value()) {
            return title ? "Is Historizing" : QVariant::fromValue(m_node.is_historizing.value());
        }
        if (attribute_id == AttributeId::EXECUTABLE && m_node.is_executable.has_value()) {
            return title ? "Is Executable" : QVariant::fromValue(m_node.is_executable.value());
        }
        if (attribute_id == AttributeId::USER_EXECUTABLE && m_node.is_user_executable.has_value()) {
            return title ? "Is User Executable" : QVariant::fromValue(m_node.is_user_executable.value());
        }

        if (attribute_id == AttributeId::VALUE && m_node.data_value.has_value()) {
            return title ? "Value" : m_node.data_value->getValue().toQVariant();
        }
        if (attribute_id == AttributeId::DATA_TYPE && m_node.data_type.has_value()) {
            return title ? "Data Type" : m_node.data_type.value()->getDisplayName().getText();
        }
        if (attribute_id == AttributeId::VALUE_RANK && m_node.value_rank.has_value()) {
            return title ? "Value Rank" : value_rank_to_string(m_node.value_rank.value());
        }
        if (attribute_id == AttributeId::ARRAY_DIMENSIONS && m_node.array_dimensions.has_value()) {
            if (sub_item == 0) {
                return title ? "Array Dimensions" : QString("[%1]").arg(m_node.array_dimensions->size());
            }

            return title ? QString("[%1]").arg(sub_item - 1)
                         : QVariant::fromValue(m_node.array_dimensions.value()[sub_item - 1]);
        }

        return {};
    }

    QVariant AttributeViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            if (section == 0) {
                return "Attribute";
            }

            if (section == 1) {
                return "Value";
            }
        }
        return {};
    }

    std::uint32_t AttributeViewModel::itemId(QModelIndex index) noexcept {
        return static_cast<std::uint32_t>(index.internalId());
    }

    constexpr std::uint32_t AttributeViewModel::itemId(opcua_qt::abstraction::AttributeId attribute,
                                                       std::uint8_t                       sub_item) noexcept {
        return itemId(static_cast<std::uint32_t>(attribute), sub_item);
    }

    constexpr std::uint32_t AttributeViewModel::itemId(std::uint32_t attribute, std::uint8_t sub_item) noexcept {
        std::uint32_t sub_item_byte = sub_item;
        sub_item_byte = sub_item_byte << 8; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

        return attribute | sub_item_byte;
    }

    constexpr opcua_qt::abstraction::AttributeId AttributeViewModel::attributeId(std::uint32_t item_id) noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        return static_cast<AttributeId>(item_id & 0xFFU);
    }

    constexpr std::uint8_t AttributeViewModel::subId(std::uint32_t item_id) noexcept {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        return static_cast<std::uint8_t>(item_id >> 8);
    }

} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
