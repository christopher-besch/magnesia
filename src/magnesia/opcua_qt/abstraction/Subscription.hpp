#pragma once

#include "AttributeId.hpp"
#include "DataValue.hpp"
#include "MonitoredItem.hpp"
#include "NodeId.hpp"
#include "SubscriptionParameters.hpp"
#include "Variant.hpp"

#include <bits/types/__sigval_t.h>

#include <open62541pp/Client.h>
#include <open62541pp/Subscription.h>
#include <open62541pp/types/Builtin.h>

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * A class for subscribing to events and attribute changes.
     *
     * @see MonitoredItem
     *
     * See https://reference.opcfoundation.org/Core/Part1/v105/docs/6.3.5.2
     */
    class Subscription : public QObject {
        Q_OBJECT

      public:
        explicit Subscription(opcua::Subscription<opcua::Client> subscription);

        /**
         * Enable or disable publishing of events and data changes.
         *
         * @param publishing whether to send events or not
         */
        void setPublishingMode(bool publishing);

        /**
         * Set new subscription parameters.
         *
         * @see SubscriptionParameters
         *
         * @param parameters the new subscription parameters
         */
        void setSubscriptionParameters(SubscriptionParameters parameters);

        /**
         * Get all currently monitored items. There is one item for every attribute and every event of every node.
         *
         * @see MonitoredItem
         */
        [[nodiscard]] QList<MonitoredItem> getMonitoredItems() noexcept;

        /**
         * Subscribe to an attribute. When it changes value, the valueChanged signal will be emitted.
         */
        MonitoredItem subscribeDataChanged(NodeId node_id, AttributeId attribute_id);

        /**
         * Subscribe to an event. When it changes value, the eventTriggered signal will be emitted.
         */
        MonitoredItem subscribeEvent(NodeId node_id);

        /**
         * Get the underlying subscription
         */
        [[nodiscard]] const opcua::Subscription<opcua::Client>& handle() const noexcept;

        /**
         * Get the underlying subscription
         */
        [[nodiscard]] opcua::Subscription<opcua::Client>& handle() noexcept;

      signals:
        void valueChanged(NodeId node_id, AttributeId attribute_id, QSharedPointer<DataValue> value);
        void eventTriggered(NodeId node_id, QSharedPointer<QList<Variant>>);

      private:
        opcua::Subscription<opcua::Client> m_subscription;
    };
} // namespace magnesia::opcua_qt::abstraction
