#include "Subscription.hpp"

#include "../../qt_version_check.hpp"
#include "AttributeId.hpp"
#include "DataValue.hpp"
#include "MonitoredItem.hpp"
#include "NodeId.hpp"
#include "SubscriptionParameters.hpp"
#include "Variant.hpp"

#include <cstdint>
#include <iterator>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Span.h>
#include <open62541pp/Subscription.h>
#include <open62541pp/types/Composed.h>
#include <open62541pp/types/DataValue.h>
#include <open62541pp/types/Variant.h>

#include <QList>
#include <QSharedPointer>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    Subscription::Subscription(opcua::Subscription<opcua::Client> subscription) : m_subscription(subscription) {}

    void Subscription::setPublishingMode(bool publishing) {
        m_subscription.setPublishingMode(publishing);
    }

    void Subscription::setSubscriptionParameters(SubscriptionParameters parameters) {
        m_subscription.setSubscriptionParameters(parameters.handle());
    }

    QList<MonitoredItem> Subscription::getMonitoredItems() noexcept {
        auto                 vector = m_subscription.getMonitoredItems();
        QList<MonitoredItem> items{std::move_iterator{vector.begin()}, std::move_iterator{vector.end()}};
        return items;
    }

    MonitoredItem Subscription::subscribeDataChanged(NodeId node_id, AttributeId attribute_id) {
        return MonitoredItem(m_subscription.subscribeDataChange(
            node_id.handle(), static_cast<opcua::AttributeId>(attribute_id),
            [&, node_id, attribute_id](uint32_t /*subId*/, uint32_t /*monId*/, const opcua::DataValue& value) {
                Q_EMIT valueChanged(node_id, attribute_id, QSharedPointer<DataValue>{new DataValue(value)});
            }));
    }

    MonitoredItem Subscription::subscribeEvent(NodeId node_id) {
        return MonitoredItem(m_subscription.subscribeEvent(
            node_id.handle(), opcua::EventFilter(),
            [&, node_id](uint32_t /*subId*/, uint32_t /*monId*/, opcua::Span<const opcua::Variant> event_fields) {
                auto* items = new QList<Variant>();
                items->reserve(static_cast<qsizetype>(event_fields.size()));

                for (const auto& item : event_fields) {
                    items->append(Variant(item));
                }
                Q_EMIT eventTriggered(node_id, QSharedPointer<QList<Variant>>{items});
            }));
    }

    const opcua::Subscription<opcua::Client>& Subscription::handle() const noexcept {
        return m_subscription;
    }

    opcua::Subscription<opcua::Client>& Subscription::handle() noexcept {
        return m_subscription;
    }
} // namespace magnesia::opcua_qt::abstraction
