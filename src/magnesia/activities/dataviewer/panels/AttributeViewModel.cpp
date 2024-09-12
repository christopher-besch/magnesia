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
#include <qtmetamacros.h>

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
using magnesia::opcua_qt::abstraction::Subscription;
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

    QVariant localized_text_data(bool title, std::uint8_t sub_id, const LocalizedText* value, const QString& name) {
        if (value == nullptr) {
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

    QVariant qualified_name_data(bool title, std::uint8_t sub_id, const QualifiedName* value, const QString& name) {
        if (value == nullptr) {
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

    QVariant array_dimensions_data(bool title, std::uint8_t sub_id, const std::vector<std::uint32_t>* value,
                                   const QString& name) {
        if (value == nullptr) {
            return {};
        }

        if (sub_id == 0) {
            return title ? name : QString("[%1]").arg(value->size());
        }

        return title ? QString("[%1]").arg(sub_id - 1) : QVariant::fromValue(value->at(sub_id - 1));
    }

    QVariant value_data(bool title, const DataValue* value, const QString& name) {
        if (value == nullptr) {
            return {};
        }

        return title ? name : value->getValue().toQVariant();
    }

    QVariant data_type_data(bool title, const std::optional<NodeId>& value, Connection* connection,
                            const QString& name) {
        if (!value.has_value() || connection == nullptr) {
            return {};
        }

        return title ? name : connection->getNode(*value)->getDisplayName().getText();
    }
} // namespace

namespace magnesia::activities::dataviewer::panels::attribute_view_panel {
    AttributeViewModel::AttributeViewModel(QObject* parent) : QAbstractItemModel(parent) {}

    void AttributeViewModel::setNode(Node* node, Connection* connection) {
        beginResetModel();

        m_connection = connection;
        m_node       = node;

        m_available_attributes.clear();

        m_available_attributes.push_back(AttributeId::NODE_ID);
        m_available_attributes.push_back(AttributeId::NODE_CLASS);
        m_available_attributes.push_back(AttributeId::BROWSE_NAME);
        m_available_attributes.push_back(AttributeId::DISPLAY_NAME);
        append_if(node->getDescription() != nullptr, AttributeId::DESCRIPTION, m_available_attributes);
        append_if(node->getWriteMask().has_value(), AttributeId::WRITE_MASK, m_available_attributes);
        append_if(node->getUserWriteMask().has_value(), AttributeId::USER_WRITE_MASK, m_available_attributes);
        append_if(node->isAbstract().has_value(), AttributeId::IS_ABSTRACT, m_available_attributes);
        append_if(node->isSymmetric().has_value(), AttributeId::SYMMETRIC, m_available_attributes);
        append_if(node->getInverseName() != nullptr, AttributeId::INVERSE_NAME, m_available_attributes);
        append_if(node->containsNoLoops().has_value(), AttributeId::CONTAINS_NO_LOOPS, m_available_attributes);
        append_if(node->getEventNotifierType().has_value(), AttributeId::EVENT_NOTFIER, m_available_attributes);
        append_if(node->getDataValue() != nullptr, AttributeId::VALUE, m_available_attributes);
        append_if(node->getDataType().has_value(), AttributeId::DATA_TYPE, m_available_attributes);
        append_if(node->getValueRank().has_value(), AttributeId::VALUE_RANK, m_available_attributes);
        append_if(node->getArrayDimensions() != nullptr, AttributeId::ARRAY_DIMENSIONS, m_available_attributes);
        append_if(node->getAccessLevel().has_value(), AttributeId::ACCESS_LEVEL, m_available_attributes);
        append_if(node->getUserAccessLevel().has_value(), AttributeId::USER_ACCESS_LEVEL, m_available_attributes);
        append_if(node->getMinimumSamplingInterval().has_value(), AttributeId::MINIMUM_SAMPLING_INTERVAL,
                  m_available_attributes);
        append_if(node->isHistorizing().has_value(), AttributeId::HISTORIZING, m_available_attributes);
        append_if(node->isExecutable().has_value(), AttributeId::EXECUTABLE, m_available_attributes);
        append_if(node->isUserExecutable().has_value(), AttributeId::USER_EXECUTABLE, m_available_attributes);

        if (m_subscription != nullptr) {
            m_subscription->deleteLater();
        }

        m_subscription = connection->createSubscription(node, m_available_attributes);
        m_subscription->setPublishingMode(true);
        connect(m_subscription, &Subscription::valueChanged, this, &AttributeViewModel::valueChanged);

        endResetModel();
    }

    QModelIndex AttributeViewModel::index(int row, int column, const QModelIndex& parent) const {
        if (row < 0 || column < 0) {
            return {};
        }

        std::uint32_t item = 0;

        if (!parent.isValid()) {
            item = item_id(m_available_attributes[static_cast<std::size_t>(row)], 0);
        } else if (row < 1) {
            // Sub items start at index 1
            return {};
        } else {
            auto sub_item  = static_cast<std::uint8_t>(row);
            auto attribute = m_available_attributes[static_cast<std::size_t>(parent.row())];
            item           = item_id(attribute, sub_item);
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
        auto row = std::distance(m_available_attributes.begin(), std::ranges::find(m_available_attributes, attribute));

        return createIndex(static_cast<int>(row), index.column(), item_id(attribute, 0));
    }

    int AttributeViewModel::rowCount(const QModelIndex& parent) const {
        if (!parent.isValid()) {
            return static_cast<int>(m_available_attributes.size());
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
                if (const auto* dimensions = m_node->getArrayDimensions()) {
                    return static_cast<int>(dimensions->size() + 1);
                }

                return 0;

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
                return node_id_data(title, sub_item, m_node->getNodeId(), "Node Id");
            case AttributeId::NODE_CLASS:
                return node_class_data(title, m_node->getNodeClass(), "Node Class");
            case AttributeId::BROWSE_NAME:
                return qualified_name_data(title, sub_item, &m_node->getBrowseName(), "Browse Name");
            case AttributeId::DISPLAY_NAME:
                return localized_text_data(title, sub_item, &m_node->getDisplayName(), "Display Name");
            case AttributeId::DESCRIPTION:
                return localized_text_data(title, sub_item, m_node->getDescription(), "Description");
            case AttributeId::WRITE_MASK:
                return write_mask_data(title, sub_item, m_node->getWriteMask(), "Write Mask");
            case AttributeId::USER_WRITE_MASK:
                return write_mask_data(title, sub_item, m_node->getUserWriteMask(), "User Write Mask");
            case AttributeId::IS_ABSTRACT:
                return scalar_data(title, m_node->isAbstract(), "Is Abstract");
            case AttributeId::SYMMETRIC:
                return scalar_data(title, m_node->isSymmetric(), "Is Symmetric");
            case AttributeId::INVERSE_NAME:
                return localized_text_data(title, sub_item, m_node->getInverseName(), "Inverse Name");
            case AttributeId::CONTAINS_NO_LOOPS:
                return scalar_data(title, m_node->containsNoLoops(), "Contains no loops");
            case AttributeId::EVENT_NOTFIER:
                return event_notifier_data(title, sub_item, m_node->getEventNotifierType(), "Event Notifier");
            case AttributeId::VALUE:
                return value_data(title, m_node->getDataValue(), "Value");
            case AttributeId::DATA_TYPE:
                return data_type_data(title, m_node->getDataType(), m_connection, "Data Type");
            case AttributeId::VALUE_RANK:
                return value_rank_data(title, m_node->getValueRank(), "Value Rank");
            case AttributeId::ARRAY_DIMENSIONS:
                return array_dimensions_data(title, sub_item, m_node->getArrayDimensions(), "Array Dimensions");
            case AttributeId::ACCESS_LEVEL:
                return access_level_data(title, sub_item, m_node->getAccessLevel(), "Access Level");
            case AttributeId::USER_ACCESS_LEVEL:
                return access_level_data(title, sub_item, m_node->getUserAccessLevel(), "User Access Level");
            case AttributeId::MINIMUM_SAMPLING_INTERVAL:
                return scalar_data(title, m_node->getMinimumSamplingInterval(), "Minimum Sampling Interval");
            case AttributeId::HISTORIZING:
                return scalar_data(title, m_node->isHistorizing(), "Is Historizing");
            case AttributeId::EXECUTABLE:
                return scalar_data(title, m_node->isExecutable(), "Is Executable");
            case AttributeId::USER_EXECUTABLE:
                return scalar_data(title, m_node->isUserExecutable(), "Is User Executable");

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

    void AttributeViewModel::valueChanged(Node* /*node*/, AttributeId attribute_id) {
        auto iter = std::ranges::find(m_available_attributes, attribute_id);
        if (iter == m_available_attributes.cend()) {
            // TODO: Find out why this happens and put an assert here
            return;
        }

        auto row         = static_cast<int>(std::distance(m_available_attributes.begin(), iter));
        auto left_index  = index(row, 0);
        auto right_index = index(row, columnCount() - 1);

        Q_EMIT dataChanged(left_index, right_index, {Qt::DisplayRole});
    }
} // namespace magnesia::activities::dataviewer::panels::attribute_view_panel
