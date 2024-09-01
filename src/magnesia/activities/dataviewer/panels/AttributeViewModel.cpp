#include "AttributeViewModel.hpp"

#include "../../../opcua_qt/Connection.hpp"
#include "../../../opcua_qt/abstraction/AccessLevel.hpp"
#include "../../../opcua_qt/abstraction/AccessLevelBitmask.hpp"
#include "../../../opcua_qt/abstraction/AttributeId.hpp"
#include "../../../opcua_qt/abstraction/DataValue.hpp"
#include "../../../opcua_qt/abstraction/EventNotifier.hpp"
#include "../../../opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "../../../opcua_qt/abstraction/LocalizedText.hpp"
#include "../../../opcua_qt/abstraction/NodeClass.hpp"
#include "../../../opcua_qt/abstraction/NodeId.hpp"
#include "../../../opcua_qt/abstraction/QualifiedName.hpp"
#include "../../../opcua_qt/abstraction/ValueRank.hpp"
#include "../../../opcua_qt/abstraction/WriteMask.hpp"
#include "../../../opcua_qt/abstraction/WriteMaskBitmask.hpp"
#include "../../../opcua_qt/abstraction/node/Node.hpp"
#include "../../../qt_version_check.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <Qt>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

using magnesia::opcua_qt::Connection;
using magnesia::opcua_qt::abstraction::access_level_to_string;
using magnesia::opcua_qt::abstraction::AccessLevelBitmask;
using magnesia::opcua_qt::abstraction::AttributeId;
using magnesia::opcua_qt::abstraction::DataValue;
using magnesia::opcua_qt::abstraction::event_notifier_to_string;
using magnesia::opcua_qt::abstraction::EventNotifierBitmask;
using magnesia::opcua_qt::abstraction::LocalizedText;
using magnesia::opcua_qt::abstraction::Node;
using magnesia::opcua_qt::abstraction::node_class_to_string;
using magnesia::opcua_qt::abstraction::NodeClass;
using magnesia::opcua_qt::abstraction::NodeId;
using magnesia::opcua_qt::abstraction::QualifiedName;
using magnesia::opcua_qt::abstraction::value_rank_to_string;
using magnesia::opcua_qt::abstraction::ValueRank;
using magnesia::opcua_qt::abstraction::write_mask_to_string;
using magnesia::opcua_qt::abstraction::WriteMaskBitmask;

namespace {
    constexpr int          c_event_notifier_count = 4;
    constexpr int          c_access_level_count   = 8;
    constexpr int          c_write_mask_count     = 26;
    constexpr unsigned int c_sub_id_size          = 8;
    constexpr unsigned int c_attribute_id_mask    = 0xFF;

    void append_if(bool append, AttributeId value, std::vector<AttributeId>& target) {
        if (!append) {
            return;
        }

        target.push_back(value);
    }

    constexpr AttributeId attribute_id(std::uint32_t item_id) noexcept {
        return static_cast<AttributeId>(item_id & c_attribute_id_mask);
    }

    // TODO: Replace with a sound solution.
    // This is bug prone because AttributeId is 32 bits wide but 8 bits are overlaid onto it.
    constexpr std::uint32_t item_id(AttributeId attribute, std::uint8_t sub_item) noexcept {
        return static_cast<std::uint32_t>(attribute) | static_cast<std::uint32_t>(sub_item) << c_sub_id_size;
    }

    constexpr std::uint8_t sub_id(std::uint32_t item_id) noexcept {
        return static_cast<std::uint8_t>(item_id >> c_sub_id_size);
    }

    std::uint32_t item_id(QModelIndex index) noexcept {
        return static_cast<std::uint32_t>(index.internalId());
    }

    template<typename T>
    QVariant scalar_data(bool title, std::optional<T> value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        return title ? name : QVariant::fromValue(*value);
    }

    QVariant node_class_data(bool title, std::optional<NodeClass> value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        return title ? name : node_class_to_string(*value);
    }

    QVariant value_rank_data(bool title, std::optional<ValueRank> value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        return title ? name : value_rank_to_string(*value);
    }

    QVariant node_id_data(bool title, std::uint8_t sub_id, const std::optional<NodeId>& value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        switch (sub_id) {
            case 0:
                return title ? name : value->toString();
            case 1:
                return title ? "Namespace Index" : QVariant::fromValue(value->getNamespaceIndex());
            case 2:
                return title ? "Identifier" : value->getIdentifier();
            default:
                Q_ASSERT(false && "Invalid sub id");
                return {};
        }
    }

    QVariant localized_text_data(bool title, std::uint8_t sub_id, const std::optional<LocalizedText>& value,
                                 const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        switch (sub_id) {
            case 0:
                return title ? name : value->getText();
            case 1:
                return title ? "Text" : value->getText();
            case 2:
                return title ? "Locale" : value->getLocale();
            default:
                Q_ASSERT(false && "Invalid sub id");
                return {};
        }
    }

    QVariant qualified_name_data(bool title, std::uint8_t sub_id, const std::optional<QualifiedName>& value,
                                 const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        switch (sub_id) {
            case 0:
                return title ? name : value->getName();
            case 1:
                return title ? "Namespace Index" : QVariant::fromValue(value->getNameSpaceIndex());
            case 2:
                return title ? "Name" : value->getName();
            default:
                Q_ASSERT(false && "Invalid sub id");
                return {};
        }
    }

    QVariant write_mask_data(bool title, std::uint8_t sub_id, std::optional<WriteMaskBitmask> value,
                             const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        if (sub_id == 0) {
            return title ? name : QVariant();
        }

        auto pair = value->getFlags().at(sub_id - 1);
        return title ? write_mask_to_string(pair.first) : QVariant::fromValue(pair.second);
    }

    QVariant access_level_data(bool title, std::uint8_t sub_id, std::optional<AccessLevelBitmask> value,
                               const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        if (sub_id == 0) {
            return title ? name : QVariant();
        }

        auto pair = value->getFlags().at(sub_id - 1);
        return title ? access_level_to_string(pair.first) : QVariant::fromValue(pair.second);
    }

    QVariant event_notifier_data(bool title, std::uint8_t sub_id, std::optional<EventNotifierBitmask> value,
                                 const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        if (sub_id == 0) {
            return title ? name : QVariant();
        }

        auto pair = value->getFlags().at(sub_id - 1);
        return title ? event_notifier_to_string(pair.first) : QVariant::fromValue(pair.second);
    }

    QVariant array_dimensions_data(bool title, std::uint8_t sub_id,
                                   const std::optional<std::vector<std::uint32_t>>& value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        if (sub_id == 0) {
            return title ? name : QString("[%1]").arg(value->size());
        }

        return title ? QString("[%1]").arg(sub_id - 1) : QVariant::fromValue(value->at(sub_id - 1));
    }

    QVariant value_data(bool title, const std::optional<DataValue>& value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        return title ? name : value->getValue().toQVariant();
    }

    QVariant data_type_data(bool title, std::optional<Node*> value, const QString& name) {
        if (!value.has_value()) {
            return {};
        }

        return title ? name : (*value)->getDisplayName().getText();
    }
} // namespace

namespace {
    template<typename T>
    std::optional<T> get_opt(T* val) {
        if (val) {
            return *val;
        }
        return std::nullopt;
    }
} // namespace

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    AttributeViewModel::AttributeViewModel(QObject* parent) : QAbstractItemModel(parent), m_node() {}

    void AttributeViewModel::setNode(Node* node, Connection* connection) {
        beginResetModel();

        std::optional<Node*> type_node = std::nullopt;
        auto                 type_id   = node->getDataType();

        if (type_id.has_value()) {
            type_node = connection->getNode(*type_id);
        }

        m_node = {
            .node_id                   = node->getNodeId(),
            .node_class                = node->getNodeClass(),
            .browse_name               = node->getBrowseName(),
            .display_name              = node->getDisplayName(),
            .description               = get_opt(node->getDescription()),
            .write_mask                = node->getWriteMask(),
            .user_write_mask           = node->getUserWriteMask(),
            .is_abstract               = node->isAbstract(),
            .is_symmetric              = node->isSymmetric(),
            .inverse_name              = get_opt(node->getInverseName()),
            .contains_no_loops         = node->containsNoLoops(),
            .event_notifier            = node->getEventNotifierType(),
            .data_value                = get_opt(node->getDataValue()),
            .data_type                 = type_node,
            .value_rank                = node->getValueRank(),
            .array_dimensions          = get_opt(node->getArrayDimensions()),
            .access_level              = node->getAccessLevel(),
            .user_access_level         = node->getUserAccessLevel(),
            .minimum_sampling_interval = node->getMinimumSamplingInterval(),
            .is_historizing            = node->isHistorizing(),
            .is_executable             = node->isExecutable(),
            .is_user_executable        = node->isUserExecutable(),
        };

        m_available_attributes.clear();

        append_if(m_node.node_id.has_value(), AttributeId::NODE_ID, m_available_attributes);
        append_if(m_node.node_class.has_value(), AttributeId::NODE_CLASS, m_available_attributes);
        append_if(m_node.browse_name.has_value(), AttributeId::BROWSE_NAME, m_available_attributes);
        append_if(m_node.display_name.has_value(), AttributeId::DISPLAY_NAME, m_available_attributes);
        append_if(m_node.description.has_value(), AttributeId::DESCRIPTION, m_available_attributes);
        append_if(m_node.write_mask.has_value(), AttributeId::WRITE_MASK, m_available_attributes);
        append_if(m_node.user_write_mask.has_value(), AttributeId::USER_WRITE_MASK, m_available_attributes);
        append_if(m_node.is_abstract.has_value(), AttributeId::IS_ABSTRACT, m_available_attributes);
        append_if(m_node.is_symmetric.has_value(), AttributeId::SYMMETRIC, m_available_attributes);
        append_if(m_node.inverse_name.has_value(), AttributeId::INVERSE_NAME, m_available_attributes);
        append_if(m_node.contains_no_loops.has_value(), AttributeId::CONTAINS_NO_LOOPS, m_available_attributes);
        append_if(m_node.event_notifier.has_value(), AttributeId::EVENT_NOTFIER, m_available_attributes);
        append_if(m_node.data_value.has_value(), AttributeId::VALUE, m_available_attributes);
        append_if(m_node.data_type.has_value(), AttributeId::DATA_TYPE, m_available_attributes);
        append_if(m_node.value_rank.has_value(), AttributeId::VALUE_RANK, m_available_attributes);
        append_if(m_node.array_dimensions.has_value(), AttributeId::ARRAY_DIMENSIONS, m_available_attributes);
        append_if(m_node.access_level.has_value(), AttributeId::ACCESS_LEVEL, m_available_attributes);
        append_if(m_node.user_access_level.has_value(), AttributeId::USER_ACCESS_LEVEL, m_available_attributes);
        append_if(m_node.minimum_sampling_interval.has_value(), AttributeId::MINIMUM_SAMPLING_INTERVAL,
                  m_available_attributes);
        append_if(m_node.is_historizing.has_value(), AttributeId::HISTORIZING, m_available_attributes);
        append_if(m_node.is_executable.has_value(), AttributeId::EXECUTABLE, m_available_attributes);
        append_if(m_node.is_user_executable.has_value(), AttributeId::USER_EXECUTABLE, m_available_attributes);

        endResetModel();
    }

    QModelIndex AttributeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (row < 1) {
            return {};
        }

        std::uint32_t item = 0;

        if (!parent.isValid()) {
            item = item_id(m_available_attributes[static_cast<std::size_t>(row - 1)], 0);
        } else {
            auto sub_item = static_cast<std::uint8_t>(row);
            item          = item_id(m_available_attributes[static_cast<std::size_t>(parent.row() - 1)], sub_item);
        }

        return createIndex(row, column, item);
    }

    QModelIndex AttributeViewModel::parent(const QModelIndex& index) const {
        if (!checkIndex(index, CheckIndexOption::DoNotUseParent | CheckIndexOption::IndexIsValid)) {
            return {};
        }

        auto item     = item_id(index);
        auto sub_item = sub_id(item);
        if (sub_item == 0) {
            return {};
        }

        auto attribute = attribute_id(item);
        auto row =
            std::distance(m_available_attributes.begin(), std::ranges::find(m_available_attributes, attribute)) + 1;

        return createIndex(static_cast<int>(row), index.column(), static_cast<std::uint32_t>(attribute));
    }

    int AttributeViewModel::rowCount(const QModelIndex& parent) const {
        if (!parent.isValid()) {
            return static_cast<int>(m_available_attributes.size() + 1);
        }

        auto item      = item_id(parent);
        auto sub_item  = sub_id(item);
        auto attribute = attribute_id(item);

        if (sub_item != 0 || std::ranges::find(m_available_attributes, attribute) == m_available_attributes.end()) {
            return 0;
        }

        switch (attribute) {
            case AttributeId::NODE_CLASS:
            case AttributeId::IS_ABSTRACT:
            case AttributeId::SYMMETRIC:
            case AttributeId::CONTAINS_NO_LOOPS:
            case AttributeId::VALUE_RANK:
            case AttributeId::MINIMUM_SAMPLING_INTERVAL:
            case AttributeId::HISTORIZING:
            case AttributeId::EXECUTABLE:
            case AttributeId::USER_EXECUTABLE:
                return 0;

            case AttributeId::NODE_ID:
            case AttributeId::BROWSE_NAME:
            case AttributeId::DISPLAY_NAME:
            case AttributeId::DESCRIPTION:
            case AttributeId::INVERSE_NAME:
                return 3;

            case AttributeId::EVENT_NOTFIER:
                return c_event_notifier_count;

            case AttributeId::ACCESS_LEVEL:
            case AttributeId::USER_ACCESS_LEVEL:
                return c_access_level_count;

            case AttributeId::WRITE_MASK:
            case AttributeId::USER_WRITE_MASK:
                return c_write_mask_count;

            case AttributeId::ARRAY_DIMENSIONS:
                if (!m_node.array_dimensions.has_value()) {
                    return 0;
                }

                return static_cast<int>(m_node.array_dimensions->size() + 1);

            case AttributeId::VALUE:
            case AttributeId::DATA_TYPE:
                // TODO: Show full structure
                return 0;

            case AttributeId::ACCESS_RESTRICTIONS:
            case AttributeId::ROLE_PERMISSIONS:
            case AttributeId::USER_ROLE_PERMISSIONS:
                // TODO: These are optional in base node class but there's no support for them in open62541pp

            case AttributeId::ACCESS_LEVEL_EX:
            case AttributeId::DATA_TYPE_DEFINITION:
                // TODO: These are not implemented in open62541pp
                Q_ASSERT(false && "Invalid attribute id");
        }

        return 0;
    }

    int AttributeViewModel::columnCount(const QModelIndex& /*parent*/) const {
        return 2;
    }

    QVariant AttributeViewModel::data(const QModelIndex& index, int role) const {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::DisplayRole) {
            return {};
        }

        const std::uint32_t item      = item_id(index);
        const std::uint8_t  sub_item  = sub_id(item);
        const AttributeId   attribute = attribute_id(item);
        const bool          title     = index.column() == 0;

        switch (attribute) {
            case AttributeId::NODE_ID:
                return node_id_data(title, sub_item, m_node.node_id, "Node Id");
            case AttributeId::NODE_CLASS:
                return node_class_data(title, m_node.node_class, "Node Class");
            case AttributeId::BROWSE_NAME:
                return qualified_name_data(title, sub_item, m_node.browse_name, "Browse Name");
            case AttributeId::DISPLAY_NAME:
                return localized_text_data(title, sub_item, m_node.display_name, "Display Name");
            case AttributeId::DESCRIPTION:
                return localized_text_data(title, sub_item, m_node.description, "Description");
            case AttributeId::WRITE_MASK:
                return write_mask_data(title, sub_item, m_node.write_mask, "Write Mask");
            case AttributeId::USER_WRITE_MASK:
                return write_mask_data(title, sub_item, m_node.user_write_mask, "User Write Mask");
            case AttributeId::IS_ABSTRACT:
                return scalar_data(title, m_node.is_abstract, "Is Abstract");
            case AttributeId::SYMMETRIC:
                return scalar_data(title, m_node.is_symmetric, "Is Symmetric");
            case AttributeId::INVERSE_NAME:
                return localized_text_data(title, sub_item, m_node.inverse_name, "Inverse Name");
            case AttributeId::CONTAINS_NO_LOOPS:
                return scalar_data(title, m_node.contains_no_loops, "Contains no loops");
            case AttributeId::EVENT_NOTFIER:
                return event_notifier_data(title, sub_item, m_node.event_notifier, "Event Notifier");
            case AttributeId::VALUE:
                return value_data(title, m_node.data_value, "Value");
            case AttributeId::DATA_TYPE:
                return data_type_data(title, m_node.data_type, "Data Type");
            case AttributeId::VALUE_RANK:
                return value_rank_data(title, m_node.value_rank, "Value Rank");
            case AttributeId::ARRAY_DIMENSIONS:
                return array_dimensions_data(title, sub_item, m_node.array_dimensions, "Array Dimensions");
            case AttributeId::ACCESS_LEVEL:
                return access_level_data(title, sub_item, m_node.access_level, "Access Level");
            case AttributeId::USER_ACCESS_LEVEL:
                return access_level_data(title, sub_item, m_node.user_access_level, "User Access Level");
            case AttributeId::MINIMUM_SAMPLING_INTERVAL:
                return scalar_data(title, m_node.minimum_sampling_interval, "Minimum Sampling Interval");
            case AttributeId::HISTORIZING:
                return scalar_data(title, m_node.is_historizing, "Is Historizing");
            case AttributeId::EXECUTABLE:
                return scalar_data(title, m_node.is_executable, "Is Executable");
            case AttributeId::USER_EXECUTABLE:
                return scalar_data(title, m_node.is_user_executable, "Is User Executable");

            case AttributeId::ACCESS_RESTRICTIONS:
            case AttributeId::ROLE_PERMISSIONS:
            case AttributeId::USER_ROLE_PERMISSIONS:
                // TODO: These are optional in base node class but there's no support for them in open62541pp

            case AttributeId::DATA_TYPE_DEFINITION:
            case AttributeId::ACCESS_LEVEL_EX:
                // TODO: These are not implemented in open62541pp
                Q_ASSERT(false && "Invalid attribute id");
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
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
