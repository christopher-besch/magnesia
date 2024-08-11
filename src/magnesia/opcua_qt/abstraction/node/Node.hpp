#pragma once

#include "../../../qt_version_check.hpp"
#include "../AccessLevelBitmask.hpp"
#include "../DataValue.hpp"
#include "../EventNotifierBitmask.hpp"
#include "../LocalizedText.hpp"
#include "../NodeClass.hpp"
#include "../NodeId.hpp"
#include "../QualifiedName.hpp"
#include "../ReferenceDescription.hpp"
#include "../ValueRank.hpp"
#include "../Variant.hpp"
#include "../WriteMaskBitmask.hpp"

#include <optional>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QList>
#include <QObject>
#include <qtmetamacros.h>

#ifdef MAGNESIA_HAS_QT_6_5
#include <QtTypes>
#else
#include <QtGlobal>
#endif

namespace magnesia::opcua_qt::abstraction {
    /**
     * Abstract base class for all Nodes.
     *
     * @see NodeClass
     * @see VariableNode
     * @see VariableTypeNode
     * @see ObjectNode
     * @see ObjectTypeNode
     * @see ReferenceTypeNode
     * @see DataTypeNode
     * @see MethodNode
     * @see ViewNode
     *
     * See https://reference.opcfoundation.org/Core/Part3/v104/docs/5
     */
    class Node : public QObject {
        Q_OBJECT

      public:
        /**
         * Get the unique id of this node.
         */
        [[nodiscard]] NodeId getNodeId() const;

        /**
         * Get the node class of this node.
         */
        [[nodiscard]] NodeClass getNodeClass();

        /**
         * Get the browse name of this node. It's a unique identifier within the namespace.
         */
        [[nodiscard]] QualifiedName getBrowseName();

        /**
         * Get the display name of this node. This is for a user to see.
         */
        [[nodiscard]] LocalizedText getDisplayName();

        /**
         * Get the description of this node. This is for a user to see.
         */
        [[nodiscard]] std::optional<LocalizedText> getDescription();

        /**
         * Get the write mask of this node. It tells, what attributes can be written to.
         */
        [[nodiscard]] std::optional<WriteMaskBitmask> getWriteMask();

        /**
         * Get the user write mask of this node. It tells, what attributes the current user can write to.
         */
        [[nodiscard]] std::optional<WriteMaskBitmask> getUserWriteMask();

        /**
         * Get the parent node of this node.
         */
        [[nodiscard]] Node* getParent();

        /**
         * Get the child nodes of this node.
         */
        [[nodiscard]] QList<Node*> getChildren();

        /**
         * Get all references to and from this node.
         */
        [[nodiscard]] QList<ReferenceDescription> getReferences();

        // TODO: RolePermissions, UserRolePermissions, AccessRestrictions
        // These are optional

        /**
         * Get the inverse name of this reference type.
         * Only applicable to RefrenceType.
         */
        [[nodiscard]] virtual std::optional<LocalizedText> getInverseName();

        /**
         * Get if this type is abstract.
         * Only applicable to VariableType, ObjectType, ReferenceType and DataType
         */
        [[nodiscard]] virtual std::optional<bool> isAbstract();

        /**
         * Get if this reference type is symmetric.
         * Only applicable to ReferenceType
         */
        [[nodiscard]] virtual std::optional<bool> isSymmetric();

        /**
         * Get if this view contains no loops.
         * Only applicable to View.
         */
        [[nodiscard]] virtual std::optional<bool> containsNoLoops();

        /**
         * Get this view's event notifier type.
         * Only applicable to View.
         */
        [[nodiscard]] virtual std::optional<EventNotifierBitmask> getEventNotifierType();

        /**
         * Get the data value for this variable or variable type.
         * Only applicable to Variable and VariableType.
         */
        [[nodiscard]] virtual std::optional<DataValue> getDataValue();

        /**
         * Get the data type of this variable or variable type.
         * Only applicable to Variable and VariableType.
         */
        [[nodiscard]] virtual std::optional<NodeId> getDataType();

        /**
         * Get the value rank of this variable or variable type. This depicts the dimension of the value.
         * Only applicable to Variable and VariableType.
         */
        [[nodiscard]] virtual std::optional<ValueRank> getValueRank();

        /**
         * Get the array dimensions of this variable or variable type if the value is an array.
         * Only applicable to Variable and VariableType.
         */
        [[nodiscard]] virtual std::optional<QList<quint32>> getArrayDimensions();

        /**
         * Get the access level of this variable for all users.
         * Only applicable to Variable.
         */
        [[nodiscard]] virtual std::optional<AccessLevelBitmask> getAccessLevel();

        /**
         * Get the access level of this variable for the current user.
         * Only applicable to Variable.
         */
        [[nodiscard]] virtual std::optional<AccessLevelBitmask> getUserAccessLevel();

        /**
         * Get the minimum sampling interval of this variable.
         * Only applicable to Variable.
         */
        [[nodiscard]] virtual std::optional<double> getMinimumSamplingInterval();

        /**
         * Get if this variable is historizing.
         * Only applicable to Variable.
         */
        [[nodiscard]] virtual std::optional<bool> isHistorizing();

        /**
         * Get if this method is executable by any user.
         * Only applicable to Method.
         */
        [[nodiscard]] virtual std::optional<bool> isExecutable();

        /**
         * Get if this method is executable by the current user.
         * Only applicable to Method.
         */
        [[nodiscard]] virtual std::optional<bool> isUserExecutable();

        /**
         * Set the display name of this node.
         *
         * @param name the new display name
         */
        void setDisplayName(LocalizedText& name);

        /**
         * Set the description of this node.
         *
         * @param description the new description
         */
        void setDescription(LocalizedText& description);

        /**
         * Set the write mask of this node.
         *
         * @param mask the new write mask
         */
        void setWriteMask(WriteMaskBitmask mask);

        /**
         * Set the user write mask of this node.
         *
         * @param mask the new user write mask
         */
        void setUserWriteMask(WriteMaskBitmask mask);

        /**
         * Set the inverse name of this reference type.
         * Only applicable to RefrenceType.
         *
         * @param name the new inverse name
         */
        virtual void setInverseName(LocalizedText& name);

        /**
         * Set if this type is abstract.
         * Only applicable to VariableType, ObjectType, ReferenceType and DataType
         *
         * @param abstract if this type is abstract
         */
        virtual void setAbstract(bool abstract);

        /**
         * Set if this reference type is symmetric.
         * Only applicable to ReferenceType
         *
         * @param symmetric if this reference type is symmetric
         */
        virtual void setSymmetric(bool symmetric);

        /**
         * Set this view's event notifier type.
         * Only applicable to View.
         *
         * @param type the new event notifier type
         */
        virtual void setEventNotifierType(EventNotifierBitmask type);

        /**
         * Set the data value.
         * Only applicable to Variable and VariableType.
         *
         * @param value the new data value
         */
        virtual void setDataValue(DataValue& value);

        /**
         * Set the value.
         * Only applicable to Variable and VariableType.
         *
         * @param value the variant to set
         */
        virtual void setDataValue(Variant& value);

        /**
         * Set the data type.
         * Only applicable to Variable and VariableType.
         *
         * @param data_type the new data type
         */
        virtual void setDataType(NodeId& data_type);

        /**
         * Set the value rank.
         * Only applicable to Variable and VariableType.
         *
         * @param rank the new value rank
         */
        virtual void setValueRank(ValueRank rank);

        /**
         * Set the array dimensions.
         * Only applicable to Variable and VariableType.
         *
         * @param dimensions the new array dimensions
         */
        virtual void setArrayDimensions(QList<quint32>& dimensions);

        /**
         * Set the access level.
         * Only applicable to Variable.
         *
         * @param mask the new access level
         */
        virtual void setAccessLevel(AccessLevelBitmask mask);

        /**
         * @brief Set the User Access Level.
         * Only applicable to Variable.
         *
         * @param mask the new user access level
         */
        virtual void setUserAccessLevel(AccessLevelBitmask mask);

        /**
         * Set if this variable is historizing.
         * Only applicable to Variable.
         *
         * @param historizing if this variable is historizing
         */
        virtual void setHistorizing(bool historizing);

        /**
         * Call this method.
         * Only applicable to Method.
         *
         * @param method_id what method to call
         * @param args arguments for the method
         */
        virtual QList<Variant> callMethod(NodeId& method_id, QList<Variant>& args);

        /**
         * Create a Node from a opcua Node. The returned node will be a subclass of Node, according to its node class.
         * Returns nullptr if the node has an invalid node class.
         *
         * @param node the opcua Node to wrap
         * @param parent the parent QObject
         */
        [[nodiscard]] static Node* fromOPCUANode(opcua::Node<opcua::Client> node, QObject* parent);

        [[nodiscard]] const opcua::Node<opcua::Client>& handle() const noexcept;
        [[nodiscard]] opcua::Node<opcua::Client>&       handle() noexcept;

        [[nodiscard]] bool operator==(const Node& other) const;

      protected:
        explicit Node(opcua::Node<opcua::Client> node, QObject* parent);

        const std::optional<LocalizedText>& setCacheDisplayName(std::optional<LocalizedText> display_name);
        const std::optional<DataValue>&     setCacheDataValue(std::optional<DataValue> data_value);

        [[nodiscard]] const std::optional<DataValue>& getCacheDataValue();

      private:
        // Subscription updates the cache directly to reduce network round-trips
        friend class Subscription;

        opcua::Node<opcua::Client>   m_node;
        std::optional<Node*>         m_cache_parent;
        std::optional<QList<Node*>>  m_cache_children;
        std::optional<LocalizedText> m_cache_display_name;
        std::optional<DataValue>     m_cache_data_value;
    };
} // namespace magnesia::opcua_qt::abstraction
