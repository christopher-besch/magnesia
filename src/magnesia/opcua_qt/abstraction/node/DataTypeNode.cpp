#include "DataTypeNode.hpp"

#include "../../../qt_version_check.hpp"
#include "Node.hpp"

#include <optional>
#include <utility>

#include <open62541pp/Client.h>
#include <open62541pp/Common.h>
#include <open62541pp/Node.h>

#include <QObject>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtAssert>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    DataTypeNode::DataTypeNode(opcua::Node<opcua::Client> node, QObject* parent) : Node(std::move(node), parent) {
        Q_ASSERT(handle().readNodeClass() == opcua::NodeClass::DataType);
    }

    std::optional<bool> DataTypeNode::isAbstract() {
        return handle().readIsAbstract();
    }

    void DataTypeNode::setAbstract(bool abstract) {
        handle().writeIsAbstract(abstract);
    }
} // namespace magnesia::opcua_qt::abstraction
