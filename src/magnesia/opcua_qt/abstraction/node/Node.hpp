#pragma once

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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <vector>

#include <open62541pp/Client.h>
#include <open62541pp/Node.h>

#include <QObject>
#include <qtmetamacros.h>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class Node
     * @brief Abstract base class for all Nodes.
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
        [[nodiscard]] const QualifiedName& getBrowseName();

        /**
         * Get the display name of this node. This is for a user to see.
         */
        [[nodiscard]] const LocalizedText& getDisplayName();

        /**
         * Get the description of this node. This is for a user to see.
         */
        [[nodiscard]] const LocalizedText* getDescription();

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
        [[nodiscard]] const std::vector<Node*>& getChildren();

        /**
         * Get all references to and from this node.
         */
        [[nodiscard]] const std::vector<ReferenceDescription>& getReferences();

        // TODO: RolePermissions, UserRolePermissions, AccessRestrictions
        // These are optional

        /**
         * Get the inverse name of this reference type.
         * Only applicable to RefrenceType.
         */
        [[nodiscard]] virtual const LocalizedText* getInverseName();

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
        [[nodiscard]] virtual const DataValue* getDataValue();

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
        [[nodiscard]] virtual const std::vector<std::uint32_t>* getArrayDimensions();

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
        virtual void setInverseName(const LocalizedText& name);

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
        virtual void setDataValue(const DataValue& value);

        /**
         * Set the value.
         * Only applicable to Variable and VariableType.
         *
         * @param value the variant to set
         */
        virtual void setDataValue(const Variant& value);

        /**
         * Set the data type.
         * Only applicable to Variable and VariableType.
         *
         * @param data_type the new data type
         */
        virtual void setDataType(const NodeId& data_type);

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
        virtual void setArrayDimensions(const std::vector<std::uint32_t>& dimensions);

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
        virtual std::vector<Variant> callMethod(const NodeId& method_id, const std::vector<Variant>& args);

        /**
         * Create a Node from a opcua Node. The returned node will be a subclass of Node, according to its node class.
         * Returns nullptr if the node has an invalid node class.
         *
         * @param node the opcua Node to wrap
         * @param parent the parent QObject
         */
        [[nodiscard]] static Node* fromOPCUANode(opcua::Node<opcua::Client> node, QObject* parent);

        /**
         * Retrieves the underlying node.
         * @return underlying node.
         */
        [[nodiscard]] const opcua::Node<opcua::Client>& handle() const noexcept;

        /**
         * Retrieves the underlying node.
         * @return underlying node.
         */
        [[nodiscard]] opcua::Node<opcua::Client>& handle() noexcept;

        [[nodiscard]] bool operator==(const Node& other) const;

        /**
         * Returns the children count that is cached in this node. Returns nullopt if nothing is cached.
         */
        [[nodiscard]] std::optional<std::size_t> childrenCountCached() const;

      protected:
        explicit Node(opcua::Node<opcua::Client> node, QObject* parent);

        struct Cache {
            template<typename T>
            using CacheType = std::optional<T>;

            CacheType<DataValue>                         data_value;
            CacheType<LocalizedText>                     display_name;
            CacheType<LocalizedText>                     description;
            CacheType<LocalizedText>                     inverse_name;
            CacheType<QualifiedName>                     browse_name;
            CacheType<std::vector<Node*>>                children;
            CacheType<std::vector<ReferenceDescription>> references;
            CacheType<std::vector<std::uint32_t>>        array_dimensions;
            CacheType<NodeId>                            data_type;
            CacheType<Node*>                             parent;
            CacheType<double>                            minimum_sampling_interval;
            CacheType<NodeClass>                         node_class;
            CacheType<WriteMaskBitmask>                  write_mask;
            CacheType<WriteMaskBitmask>                  user_write_mask;
            CacheType<ValueRank>                         value_rank;
            CacheType<bool>                              is_abstract;
            CacheType<bool>                              is_symmetric;
            CacheType<bool>                              contains_no_loops;
            CacheType<EventNotifierBitmask>              event_notifier;
            CacheType<AccessLevelBitmask>                access_level;
            CacheType<AccessLevelBitmask>                user_access_level;
            CacheType<bool>                              is_historizing;
            CacheType<bool>                              is_executable;
            CacheType<bool>                              is_user_executable;
        };

        template<typename CacheEntry, typename ValueType>
        void setCache(CacheEntry&& cache_entry, ValueType&& value) {
            std::invoke(std::forward<CacheEntry>(cache_entry), m_cache) = std::forward<ValueType>(value);
        }

        template<typename CacheEntry, typename Getter,
                 typename TargetType = std::remove_cvref_t<std::invoke_result_t<CacheEntry, Cache>>::value_type>
        const TargetType& wrapCache(CacheEntry&& cache_entry, Getter&& getter) {
            auto& entry = std::invoke(std::forward<CacheEntry>(cache_entry), m_cache);
            if (!entry.has_value()) {
                entry = std::invoke(std::forward<Getter>(getter));
            }

            return *entry;
        }

        template<typename CacheEntry>
        void invalidateCache(CacheEntry&& cache_entry) {
            std::invoke(std::forward<CacheEntry>(cache_entry), m_cache) = std::nullopt;
        }

      private:
        // Subscription updates the cache directly to reduce network round-trips
        friend class Subscription;
        Cache m_cache;

        opcua::Node<opcua::Client> m_node;
    };
} // namespace magnesia::opcua_qt::abstraction
