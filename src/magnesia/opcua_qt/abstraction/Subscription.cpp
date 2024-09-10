#include "Subscription.hpp"

#include "AccessLevelBitmask.hpp"
#include "AttributeId.hpp"
#include "DataValue.hpp"
#include "EventNotifierBitmask.hpp"
#include "LocalizedText.hpp"
#include "MonitoredItem.hpp"
#include "NodeClass.hpp"
#include "NodeId.hpp"
#include "QualifiedName.hpp"
#include "SubscriptionParameters.hpp"
#include "Variant.hpp"
#include "WriteMaskBitmask.hpp"
#include "node/Node.hpp"
#include "opcua_qt/abstraction/ValueRank.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Span.h>
#include <open62541pp/Subscription.h>
#include <open62541pp/types/Composed.h>
#include <open62541pp/types/DataValue.h>
#include <open62541pp/types/Variant.h>

#include <QLoggingCategory>
#include <qtmetamacros.h>

namespace {
    Q_LOGGING_CATEGORY(lc_opcua_subscription, "magnesia.opcua.subscription")
} // namespace

namespace magnesia::opcua_qt::abstraction {
    Subscription::Subscription(opcua::Subscription<opcua::Client> subscription) : m_subscription(subscription) {}

    void Subscription::setPublishingMode(bool publishing) {
        m_subscription.setPublishingMode(publishing);
    }

    void Subscription::setSubscriptionParameters(SubscriptionParameters parameters) {
        m_subscription.setSubscriptionParameters(parameters.handle());
    }

    std::vector<MonitoredItem> Subscription::getMonitoredItems() noexcept {
        auto vector = m_subscription.getMonitoredItems();
        return {vector.begin(), vector.end()};
    }

    MonitoredItem Subscription::subscribeDataChanged(Node* node, AttributeId attribute_id) {
        return MonitoredItem(m_subscription.subscribeDataChange(
            node->getNodeId().handle(), static_cast<opcua::AttributeId>(attribute_id),
            [this, node, attribute_id](std::uint32_t /*subId*/, std::uint32_t /*monId*/,
                                       const opcua::DataValue& value) {
                auto data_value = std::make_shared<DataValue>(value);
                updateNodeCache(node, attribute_id, *data_value);
                Q_EMIT valueChanged(node, attribute_id, std::move(data_value));
            }));
    }

    MonitoredItem Subscription::subscribeEvent(Node* node) {
        return MonitoredItem(m_subscription.subscribeEvent(
            node->getNodeId().handle(), opcua::EventFilter(),
            [this, node](std::uint32_t /*subId*/, std::uint32_t /*monId*/,
                         opcua::Span<const opcua::Variant> event_fields) {
                auto items = std::make_shared<std::vector<Variant>>(event_fields.begin(), event_fields.end());
                Q_EMIT eventTriggered(node, std::move(items));
            }));
    }

    void Subscription::updateNodeCache(Node* node, AttributeId attribute_id, const DataValue& value) {
        auto variant = value.getValue();

        switch (attribute_id) {
            case AttributeId::NODE_ID:
                // NodeId is the identifier for a node, therefore it will never be updated.
                return;
            case AttributeId::NODE_CLASS:
                node->setCache(&Node::Cache::node_class, variant.getScalar<NodeClass>());
                return;
            case AttributeId::BROWSE_NAME:
                node->setCache(&Node::Cache::browse_name, variant.getScalar<QualifiedName>());
                return;
            case AttributeId::DISPLAY_NAME:
                node->setCache(&Node::Cache::display_name, variant.getScalar<LocalizedText>());
                return;
            case AttributeId::DESCRIPTION:
                node->setCache(&Node::Cache::description, variant.getScalar<LocalizedText>());
                return;
            case AttributeId::WRITE_MASK:
                node->setCache(&Node::Cache::write_mask, variant.getScalar<WriteMaskBitmask>());
                return;
            case AttributeId::USER_WRITE_MASK:
                node->setCache(&Node::Cache::user_write_mask, variant.getScalar<WriteMaskBitmask>());
                return;
            case AttributeId::IS_ABSTRACT:
                node->setCache(&Node::Cache::is_abstract, variant.getScalar<bool>());
                return;
            case AttributeId::SYMMETRIC:
                node->setCache(&Node::Cache::is_symmetric, variant.getScalar<bool>());
                return;
            case AttributeId::INVERSE_NAME:
                node->setCache(&Node::Cache::inverse_name, variant.getScalar<LocalizedText>());
                return;
            case AttributeId::CONTAINS_NO_LOOPS:
                node->setCache(&Node::Cache::contains_no_loops, variant.getScalar<bool>());
                return;
            case AttributeId::EVENT_NOTFIER:
                node->setCache(&Node::Cache::event_notifier, variant.getScalar<EventNotifierBitmask>());
                return;
            case AttributeId::VALUE:
                node->setCache(&Node::Cache::data_value, value);
                return;
            case AttributeId::DATA_TYPE:
                node->setCache(&Node::Cache::data_type, variant.getScalar<NodeId>());
                return;
            case AttributeId::VALUE_RANK:
                node->setCache(&Node::Cache::value_rank, variant.getScalar<ValueRank>());
                return;
            case AttributeId::ARRAY_DIMENSIONS:
                node->setCache(&Node::Cache::array_dimensions, variant.getArray<std::uint32_t>());
                return;
            case AttributeId::ACCESS_LEVEL:
                node->setCache(&Node::Cache::access_level, variant.getScalar<AccessLevelBitmask>());
                return;
            case AttributeId::USER_ACCESS_LEVEL:
                node->setCache(&Node::Cache::user_access_level, variant.getScalar<AccessLevelBitmask>());
                return;
            case AttributeId::MINIMUM_SAMPLING_INTERVAL:
                node->setCache(&Node::Cache::minimum_sampling_interval, variant.getScalar<double>());
                return;
            case AttributeId::HISTORIZING:
                node->setCache(&Node::Cache::is_historizing, variant.getScalar<bool>());
                return;
            case AttributeId::EXECUTABLE:
                node->setCache(&Node::Cache::is_executable, variant.getScalar<bool>());
                return;
            case AttributeId::USER_EXECUTABLE:
                node->setCache(&Node::Cache::is_user_executable, variant.getScalar<bool>());
                return;
            case AttributeId::ACCESS_RESTRICTIONS:
            case AttributeId::ROLE_PERMISSIONS:
            case AttributeId::USER_ROLE_PERMISSIONS:
                // TODO: These are optional in base node class but there's no support for them in open62541pp

            case AttributeId::DATA_TYPE_DEFINITION:
            case AttributeId::ACCESS_LEVEL_EX:
                // TODO: These are not implemented in open62541pp
                return;
        }
    }

    const opcua::Subscription<opcua::Client>& Subscription::handle() const noexcept {
        return m_subscription;
    }

    opcua::Subscription<opcua::Client>& Subscription::handle() noexcept {
        return m_subscription;
    }

    Subscription::~Subscription() {
        try {
            m_subscription.deleteSubscription();
        } catch (const opcua::BadStatus& exception) {
            qCWarning(lc_opcua_subscription) << "Error while deleting Subscription" << exception.what();
        }
    }
} // namespace magnesia::opcua_qt::abstraction
