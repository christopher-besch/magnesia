#pragma once

#include "Node.hpp"
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
     * Subclass of Node for DataType NodeClass.
     *
     * @see Node
     * @see NodeClass
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5.8.3
     */
    class DataTypeNode : public Node {
      public:
        explicit DataTypeNode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] std::optional<bool> isAbstract() override;

        void setAbstract(bool abstract) override;
    };
} // namespace magnesia::opcua_qt::abstraction
