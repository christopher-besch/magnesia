#include "Subscription.hpp"

#include "../../qt_version_check.hpp"
#include "AttributeId.hpp"
#include "DataValue.hpp"
#include "LocalizedText.hpp"
#include "MonitoredItem.hpp"
#include "NodeId.hpp"
#include "SubscriptionParameters.hpp"
#include "Variant.hpp"
#include "node/Node.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/ErrorHandling.h>
#include <open62541pp/Span.h>
#include <open62541pp/Subscription.h>
#include <open62541pp/types/Builtin.h>
#include <open62541pp/types/Composed.h>
#include <open62541pp/types/DataValue.h>
#include <open62541pp/types/Variant.h>

#include <QLoggingCategory>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

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
            [this, node, attribute_id](uint32_t /*subId*/, uint32_t /*monId*/, const opcua::DataValue& value) {
                if (attribute_id == opcua_qt::abstraction::AttributeId::DISPLAY_NAME) {
                    auto text = value.getValue().getScalar<opcua::LocalizedText>();
                    node->setCacheDisplayName(LocalizedText{std::move(text)});
                } else if (attribute_id == opcua_qt::abstraction::AttributeId::VALUE) {
                    node->setCacheDataValue(DataValue(value));
                }

                Q_EMIT valueChanged(node, attribute_id, std::make_shared<DataValue>(value));
            }));
    }

    MonitoredItem Subscription::subscribeEvent(Node* node) {
        return MonitoredItem(m_subscription.subscribeEvent(
            node->getNodeId().handle(), opcua::EventFilter(),
            [this, node](uint32_t /*subId*/, uint32_t /*monId*/, opcua::Span<const opcua::Variant> event_fields) {
                auto items = std::make_shared<std::vector<Variant>>(event_fields.begin(), event_fields.end());
                Q_EMIT eventTriggered(node, std::move(items));
            }));
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
        } catch (opcua::BadStatus& exception) {
            qCWarning(lc_opcua_subscription) << "Error while deleting Subscription" << exception.what();
        }
    }
} // namespace magnesia::opcua_qt::abstraction
