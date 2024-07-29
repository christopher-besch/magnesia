#pragma once

#include "Node.hpp"
#include "opcua_qt/abstraction/EventNotifierBitmask.hpp"
#include "qt_version_check.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>
#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    /**
     * Subclass of Node for View NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.4
     */
    class ViewNode : public Node {
      public:
        explicit ViewNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<bool>                 containsNoLoops() override;
        [[nodiscard]] std::optional<EventNotifierBitmask> getEventNotifierType() override;

        void setEventNotifierType(EventNotifierBitmask type) override;
    };
} // namespace magnesia::opcua_qt::abstraction
